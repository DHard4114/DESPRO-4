package etl

import (
	"context"
	"encoding/json"
	"log"
	"sync"
	"time"

	"smart-sanitation-esos/server/models"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

type Pipeline struct {
	InStream  chan models.RawMQTTMessage
	OutStream chan models.TelemetryRecord
	DBPool    *pgxpool.Pool

	// LRU Cache untuk deduplikasi (node_code -> last_sequence_no)
	muDedup sync.RWMutex
	dedup   map[string]uint32

	// Cache resolusi NodeCode ke NodeID
	muNodeID sync.RWMutex
	nodeIDs  map[string]string
}

func NewPipeline(dbPool *pgxpool.Pool) *Pipeline {
	return &Pipeline{
		InStream:  make(chan models.RawMQTTMessage, 1000), // Buffered channel [ADR-01]
		OutStream: make(chan models.TelemetryRecord, 1000),
		DBPool:    dbPool,
		dedup:     make(map[string]uint32),
		nodeIDs:   make(map[string]string),
	}
}

// StartWorkers menjalankan pool Goroutine untuk mengolah JSON [ADR-01]
func (p *Pipeline) StartWorkers(workerCount int) {
	for i := 0; i < workerCount; i++ {
		go func(workerID int) {
			for msg := range p.InStream {
				var payload models.TelemetryPayload
				if err := json.Unmarshal(msg.Payload, &payload); err != nil {
					log.Printf("Worker %d: Gagal parse JSON dari topic %s: %v", workerID, msg.Topic, err)
					continue
				}

				// Deduplikasi [ADR-01]
				p.muDedup.RLock()
				lastSeq, exists := p.dedup[payload.NodeCode]
				p.muDedup.RUnlock()

				if exists && payload.SequenceNo <= lastSeq {
					// Drop pesan duplikat dari QoS 1
					continue
				}

				p.muDedup.Lock()
				p.dedup[payload.NodeCode] = payload.SequenceNo
				p.muDedup.Unlock()

				// Resolusi NodeCode -> NodeID menggunakan Cache Internal
				p.muNodeID.RLock()
				nodeID, hasNodeID := p.nodeIDs[payload.NodeCode]
				p.muNodeID.RUnlock()

				if !hasNodeID {
					ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
					err := p.DBPool.QueryRow(ctx, "SELECT node_id FROM sanitation_nodes WHERE node_code = $1", payload.NodeCode).Scan(&nodeID)
					cancel()
					if err != nil {
						log.Printf("Worker %d: NodeCode %s tidak terdaftar di DB: %v", workerID, payload.NodeCode, err)
						continue // Drop pesan jika node tidak dikenal
					}
					p.muNodeID.Lock()
					p.nodeIDs[payload.NodeCode] = nodeID
					p.muNodeID.Unlock()
				}

				// Cek anomali (Threshold Cache) [ADR-07]
				// Di sini logika anomaly detection memicu Insert ke incident_alerts
				// ...

				receivedTime := time.Now()
				if payload.Timestamp > 0 {
					receivedTime = time.Unix(int64(payload.Timestamp), 0)
				}

				record := models.TelemetryRecord{
					NodeID:         nodeID,
					NodeCode:       payload.NodeCode,
					SequenceNo:     payload.SequenceNo,
					WaterLevelCM:   payload.WaterLevelCM,
					AmmoniaPPM:     payload.AmmoniaPPM,
					H2SPPM:         payload.H2SPPM,
					BatteryVoltage: payload.BatteryVoltage,
					SOSTriggered:   payload.SOSTriggered == 1,
					ReceivedAt:     receivedTime,
				}

				p.OutStream <- record
			}
		}(i)
	}
	log.Printf("ETL Pipeline: %d workers siap beroperasi.", workerCount)
}

// StartBatchInserter menjalankan Micro-batch insert ke PostgreSQL
func (p *Pipeline) StartBatchInserter() {
	go func() {
		buffer := make([]models.TelemetryRecord, 0, 50)
		ticker := time.NewTicker(3 * time.Second)
		defer ticker.Stop()

		for {
			select {
			case record := <-p.OutStream:
				buffer = append(buffer, record)
				if len(buffer) >= 50 {
					p.flushBuffer(&buffer)
				}
			case <-ticker.C:
				if len(buffer) > 0 {
					p.flushBuffer(&buffer)
				}
			}
		}
	}()
}

func (p *Pipeline) flushBuffer(buffer *[]models.TelemetryRecord) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	rows := make([][]interface{}, 0, len(*buffer))
	for _, rec := range *buffer {
		rows = append(rows, []interface{}{
			rec.NodeID,       // UUIDv7 string
			rec.NodeCode,
			rec.SequenceNo,
			rec.WaterLevelCM,
			rec.AmmoniaPPM,
			rec.H2SPPM,
			rec.BatteryVoltage,
			rec.SOSTriggered, // pgx driver mengkonversi bool ke BOOLEAN di postgres
			rec.ReceivedAt,
		})
	}

	_, err := p.DBPool.CopyFrom(
		ctx,
		pgx.Identifier{"telemetry_records"},
		[]string{"node_id", "node_code", "sequence_no", "water_level_cm", "ammonia_ppm", "h2s_ppm", "battery_voltage", "sos_triggered", "received_at"},
		pgx.CopyFromRows(rows),
	)

	if err != nil {
		log.Printf("ETL Batch Insert GAGAL: %v", err)
	} else {
		log.Printf("ETL Batch Insert SUKSES: %d records", len(*buffer))
	}

	// Reset buffer
	*buffer = (*buffer)[:0]
}
