package database

import (
	"database/sql"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"time"

	"github.com/google/uuid"
	_ "modernc.org/sqlite"
	"esos-server/models"
)

type DB struct {
	conn *sql.DB
}

// InitDB initializes SQLite database with optimized WAL mode and UUID primary key tables
func InitDB(dbPath string) (*DB, error) {
	// Ensure directory exists
	dir := filepath.Dir(dbPath)
	if dir != "" && dir != "." {
		if err := os.MkdirAll(dir, 0755); err != nil {
			return nil, fmt.Errorf("failed to create db directory: %w", err)
		}
	}

	// Open connection with Write-Ahead Logging (WAL) for high concurrency
	dsn := fmt.Sprintf("%s?_pragma=busy_timeout(5000)&_pragma=journal_mode(WAL)&_pragma=synchronous(NORMAL)", dbPath)
	conn, err := sql.Open("sqlite", dsn)
	if err != nil {
		return nil, fmt.Errorf("failed to open database: %w", err)
	}

	conn.SetMaxOpenConns(1) // SQLite works best with 1 writer connection
	conn.SetMaxIdleConns(1)

	db := &DB{conn: conn}
	if err := db.migrate(); err != nil {
		return nil, fmt.Errorf("failed to run migrations: %w", err)
	}

	log.Printf("[DATABASE] SQLite connected successfully at %s (WAL Mode & UUID Keys Active)", dbPath)
	return db, nil
}

func (d *DB) Close() error {
	return d.conn.Close()
}

func (d *DB) migrate() error {
	schema := `
	CREATE TABLE IF NOT EXISTS sanitation_nodes (
		node_id TEXT PRIMARY KEY,
		node_code TEXT NOT NULL UNIQUE,
		location_name TEXT NOT NULL,
		latitude REAL,
		longitude REAL,
		installation_date DATETIME DEFAULT CURRENT_TIMESTAMP,
		status TEXT DEFAULT 'ACTIVE'
	);

	CREATE TABLE IF NOT EXISTS telemetry_records (
		record_id TEXT PRIMARY KEY,
		node_id TEXT NOT NULL,
		sequence_no INTEGER NOT NULL,
		water_level_cm REAL NOT NULL,
		water_volume_percentage REAL NOT NULL,
		ammonia_ppm REAL NOT NULL,
		h2s_ppm REAL NOT NULL,
		battery_voltage REAL NOT NULL,
		battery_percentage REAL NOT NULL,
		solar_charging_active BOOLEAN DEFAULT 1,
		valve_servo_open BOOLEAN DEFAULT 0,
		sos_button_triggered BOOLEAN DEFAULT 0,
		air_quality_index TEXT DEFAULT 'GOOD',
		anomaly_detected BOOLEAN DEFAULT 0,
		rssi_dbm INTEGER,
		snr_db REAL,
		received_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (node_id) REFERENCES sanitation_nodes(node_id)
	);

	CREATE TABLE IF NOT EXISTS incident_alerts (
		alert_id TEXT PRIMARY KEY,
		node_id TEXT NOT NULL,
		alert_type TEXT NOT NULL,
		severity TEXT NOT NULL,
		description TEXT NOT NULL,
		is_resolved BOOLEAN DEFAULT 0,
		resolved_at DATETIME,
		resolved_by TEXT,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (node_id) REFERENCES sanitation_nodes(node_id)
	);

	CREATE INDEX IF NOT EXISTS idx_telemetry_node_time ON telemetry_records (node_id, received_at DESC);
	CREATE INDEX IF NOT EXISTS idx_alerts_unresolved ON incident_alerts (is_resolved, created_at DESC);

	INSERT OR IGNORE INTO sanitation_nodes (node_id, node_code, location_name, latitude, longitude, status)
	VALUES 
		('a0000000-0000-0000-0000-000000000001', 'NODE_SANITATION_01', 'Shelter Posko A - Zona Evakuasi 1', -6.362141, 106.824964, 'ACTIVE'),
		('a0000000-0000-0000-0000-000000000002', 'NODE_SANITATION_02', 'Shelter Posko B - Dapur Umum & Medis', -6.363025, 106.825830, 'ACTIVE');
	`
	_, err := d.conn.Exec(schema)
	return err
}

// BatchInsertTelemetry performs high-speed bulk insertions inside a single ACID transaction with UUID primary keys
func (d *DB) BatchInsertTelemetry(records []models.TransformedTelemetry) error {
	if len(records) == 0 {
		return nil
	}

	tx, err := d.conn.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	stmt, err := tx.Prepare(`
		INSERT INTO telemetry_records (
			record_id, node_id, sequence_no, water_level_cm, water_volume_percentage,
			ammonia_ppm, h2s_ppm, battery_voltage, battery_percentage,
			solar_charging_active, valve_servo_open, sos_button_triggered,
			air_quality_index, anomaly_detected, rssi_dbm, snr_db, received_at
		) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
	`)
	if err != nil {
		return err
	}
	defer stmt.Close()

	for _, r := range records {
		recID := r.RecordID
		if recID == "" {
			recID = uuid.New().String()
		}

		_, err := stmt.Exec(
			recID, r.NodeID, r.SequenceNo, r.WaterLevelCm, r.WaterVolumePct,
			r.AmmoniaPpm, r.H2sPpm, r.BatteryVoltage, r.BatteryPercentage,
			r.SolarChargingActive, r.ValveServoOpen, r.SosButtonTriggered,
			r.AirQualityIndex, r.AnomalyDetected, r.RssiDbm, r.SnrDb, r.ReceivedAt,
		)
		if err != nil {
			return err
		}
	}

	return tx.Commit()
}

// InsertAlert records emergency and threshold alerts with UUID Primary Key
func (d *DB) InsertAlert(alert models.IncidentAlert) (string, error) {
	alertID := alert.AlertID
	if alertID == "" {
		alertID = uuid.New().String()
	}

	_, err := d.conn.Exec(`
		INSERT INTO incident_alerts (alert_id, node_id, alert_type, severity, description, is_resolved, created_at)
		VALUES (?, ?, ?, ?, ?, 0, ?)
	`, alertID, alert.NodeID, alert.AlertType, alert.Severity, alert.Description, alert.CreatedAt)
	if err != nil {
		return "", err
	}
	return alertID, nil
}

// GetLatestTelemetry queries the latest sensor snapshot per node
func (d *DB) GetLatestTelemetry(nodeID string) ([]models.TransformedTelemetry, error) {
	var query string
	var args []interface{}

	if nodeID != "" {
		query = `
			SELECT record_id, node_id, sequence_no, water_level_cm, water_volume_percentage,
			       ammonia_ppm, h2s_ppm, battery_voltage, battery_percentage,
			       solar_charging_active, valve_servo_open, sos_button_triggered,
			       air_quality_index, anomaly_detected, rssi_dbm, snr_db, received_at
			FROM telemetry_records
			WHERE node_id = ?
			ORDER BY received_at DESC LIMIT 1
		`
		args = append(args, nodeID)
	} else {
		query = `
			SELECT t1.record_id, t1.node_id, t1.sequence_no, t1.water_level_cm, t1.water_volume_percentage,
			       t1.ammonia_ppm, t1.h2s_ppm, t1.battery_voltage, t1.battery_percentage,
			       t1.solar_charging_active, t1.valve_servo_open, t1.sos_button_triggered,
			       t1.air_quality_index, t1.anomaly_detected, t1.rssi_dbm, t1.snr_db, t1.received_at
			FROM telemetry_records t1
			INNER JOIN (
				SELECT node_id, MAX(received_at) as max_time
				FROM telemetry_records GROUP BY node_id
			) t2 ON t1.node_id = t2.node_id AND t1.received_at = t2.max_time
		`
	}

	rows, err := d.conn.Query(query, args...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var results []models.TransformedTelemetry
	for rows.Next() {
		var r models.TransformedTelemetry
		err := rows.Scan(
			&r.RecordID, &r.NodeID, &r.SequenceNo, &r.WaterLevelCm, &r.WaterVolumePct,
			&r.AmmoniaPpm, &r.H2sPpm, &r.BatteryVoltage, &r.BatteryPercentage,
			&r.SolarChargingActive, &r.ValveServoOpen, &r.SosButtonTriggered,
			&r.AirQualityIndex, &r.AnomalyDetected, &r.RssiDbm, &r.SnrDb, &r.ReceivedAt,
		)
		if err != nil {
			return nil, err
		}
		results = append(results, r)
	}
	return results, nil
}

// GetHistoricalTelemetry queries time-series history for charts
func (d *DB) GetHistoricalTelemetry(nodeID string, limit int) ([]models.TransformedTelemetry, error) {
	if limit <= 0 || limit > 500 {
		limit = 50
	}
	query := `
		SELECT record_id, node_id, sequence_no, water_level_cm, water_volume_percentage,
		       ammonia_ppm, h2s_ppm, battery_voltage, battery_percentage,
		       solar_charging_active, valve_servo_open, sos_button_triggered,
		       air_quality_index, anomaly_detected, rssi_dbm, snr_db, received_at
		FROM telemetry_records
		WHERE (? = '' OR node_id = ?)
		ORDER BY received_at DESC LIMIT ?
	`
	rows, err := d.conn.Query(query, nodeID, nodeID, limit)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var results []models.TransformedTelemetry
	for rows.Next() {
		var r models.TransformedTelemetry
		err := rows.Scan(
			&r.RecordID, &r.NodeID, &r.SequenceNo, &r.WaterLevelCm, &r.WaterVolumePct,
			&r.AmmoniaPpm, &r.H2sPpm, &r.BatteryVoltage, &r.BatteryPercentage,
			&r.SolarChargingActive, &r.ValveServoOpen, &r.SosButtonTriggered,
			&r.AirQualityIndex, &r.AnomalyDetected, &r.RssiDbm, &r.SnrDb, &r.ReceivedAt,
		)
		if err != nil {
			return nil, err
		}
		results = append(results, r)
	}
	return results, nil
}

// GetActiveAlerts queries unresolved emergency incidents
func (d *DB) GetActiveAlerts() ([]models.IncidentAlert, error) {
	rows, err := d.conn.Query(`
		SELECT alert_id, node_id, alert_type, severity, description, is_resolved, resolved_at, resolved_by, created_at
		FROM incident_alerts
		WHERE is_resolved = 0
		ORDER BY created_at DESC
	`)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var alerts []models.IncidentAlert
	for rows.Next() {
		var a models.IncidentAlert
		err := rows.Scan(
			&a.AlertID, &a.NodeID, &a.AlertType, &a.Severity, &a.Description,
			&a.IsResolved, &a.ResolvedAt, &a.ResolvedBy, &a.CreatedAt,
		)
		if err != nil {
			return nil, err
		}
		alerts = append(alerts, a)
	}
	return alerts, nil
}

// ResolveAlert marks an alert as resolved by UUID
func (d *DB) ResolveAlert(alertID string, resolvedBy string) error {
	now := time.Now()
	_, err := d.conn.Exec(`
		UPDATE incident_alerts
		SET is_resolved = 1, resolved_at = ?, resolved_by = ?
		WHERE alert_id = ?
	`, now, resolvedBy, alertID)
	return err
}
