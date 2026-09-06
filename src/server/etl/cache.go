package etl

import (
	"context"
	"encoding/json"
	"log"
	"sync"
	"time"

	"smart-sanitation-esos/server/models"
	"github.com/jackc/pgx/v5/pgxpool"
)

// ThresholdCache menampung ambang batas secara in-memory untuk performa ETL [ADR-07]
type ThresholdCache struct {
	mu   sync.RWMutex
	// map key format: "nodeID_metricName"
	data map[string]models.ThresholdConfig
}

var GlobalThresholdCache = &ThresholdCache{
	data: make(map[string]models.ThresholdConfig),
}

// LoadAll membaca semua threshold dari PostgreSQL saat bootstrap
func (c *ThresholdCache) LoadAll(dbPool *pgxpool.Pool) error {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	rows, err := dbPool.Query(ctx, "SELECT node_id, metric_name, warning_value, critical_value FROM node_threshold_configs")
	if err != nil {
		return err
	}
	defer rows.Close()

	c.mu.Lock()
	defer c.mu.Unlock()

	for rows.Next() {
		var cfg models.ThresholdConfig
		if err := rows.Scan(&cfg.NodeID, &cfg.MetricName, &cfg.WarningValue, &cfg.CriticalValue); err != nil {
			return err
		}
		key := cfg.NodeID + "_" + cfg.MetricName
		c.data[key] = cfg
	}
	return nil
}

// Get mengembalikan ThresholdConfig secara thread-safe
func (c *ThresholdCache) Get(nodeID, metricName string) (models.ThresholdConfig, bool) {
	c.mu.RLock()
	defer c.mu.RUnlock()
	cfg, exists := c.data[nodeID+"_"+metricName]
	return cfg, exists
}

// StartListenNotify menjalankan Goroutine untuk mendengarkan perubahan threshold dari DB
func (c *ThresholdCache) StartListenNotify(dbPool *pgxpool.Pool) {
	go func() {
		ctx := context.Background()
		conn, err := dbPool.Acquire(ctx)
		if err != nil {
			log.Fatalf("ThresholdCache: Gagal koneksi Listen/Notify: %v", err)
		}
		defer conn.Release()

		_, err = conn.Exec(ctx, "LISTEN threshold_config_updated")
		if err != nil {
			log.Fatalf("ThresholdCache: Gagal LISTEN: %v", err)
		}

		log.Println("ThresholdCache: Berhasil Listen ke 'threshold_config_updated'")

		for {
			notification, err := conn.Conn().WaitForNotification(ctx)
			if err != nil {
				log.Printf("ThresholdCache: Error WaitForNotification: %v\n", err)
				time.Sleep(2 * time.Second)
				continue
			}

			// Parse JSON payload dari trigger PostgreSQL
			var payload struct {
				Operation string `json:"operation"`
				NodeID    string `json:"node_id"`
				Metric    string `json:"metric"`
			}
			if err := json.Unmarshal([]byte(notification.Payload), &payload); err != nil {
				log.Printf("ThresholdCache: Gagal parse payload NOTIFY: %v\n", err)
				continue
			}

			// Jika UPDATE/INSERT, ambil data terbaru dan perbarui map thread-safe
			if payload.Operation == "UPDATE" || payload.Operation == "INSERT" {
				var cfg models.ThresholdConfig
				err = dbPool.QueryRow(ctx, "SELECT warning_value, critical_value FROM node_threshold_configs WHERE node_id=$1 AND metric_name=$2", payload.NodeID, payload.Metric).Scan(&cfg.WarningValue, &cfg.CriticalValue)
				if err == nil {
					cfg.NodeID = payload.NodeID
					cfg.MetricName = payload.Metric
					c.mu.Lock()
					c.data[payload.NodeID+"_"+payload.Metric] = cfg
					c.mu.Unlock()
					log.Printf("ThresholdCache: Cache di-update untuk %s (%s)", payload.NodeID, payload.Metric)
				}
			}
		}
	}()
}
