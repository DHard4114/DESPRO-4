-- ==========================================================
-- SMART-SANITATION eSOS SQLITE / POSTGRESQL DATABASE SCHEMA
-- Purpose: Telemetry, Alert Events, Node Status, and Audit Logs
-- ==========================================================

-- 1. Nodes Table: Registry of Active Sanitation Stations
CREATE TABLE IF NOT EXISTS sanitation_nodes (
    node_id VARCHAR(32) PRIMARY KEY,
    location_name VARCHAR(128) NOT NULL,
    latitude DECIMAL(10, 7),
    longitude DECIMAL(10, 7),
    installation_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(16) DEFAULT 'ACTIVE' -- ACTIVE, INACTIVE, MAINTENANCE, ALERT
);

-- 2. Telemetry Readings Table: High-Frequency Sensor Stream
CREATE TABLE IF NOT EXISTS telemetry_records (
    record_id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id VARCHAR(32) NOT NULL,
    sequence_no INTEGER NOT NULL,
    water_level_cm REAL NOT NULL,
    water_volume_percentage REAL,
    ammonia_ppm REAL NOT NULL,
    h2s_ppm REAL NOT NULL,
    battery_voltage REAL NOT NULL,
    battery_percentage REAL,
    solar_charging_active BOOLEAN DEFAULT 1,
    valve_servo_open BOOLEAN DEFAULT 0,
    sos_button_triggered BOOLEAN DEFAULT 0,
    rssi_dbm INTEGER,
    snr_db REAL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (node_id) REFERENCES sanitation_nodes(node_id)
);

-- 3. Incident Alert Logs: Emergency Events & Threshold Violations
CREATE TABLE IF NOT EXISTS incident_alerts (
    alert_id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id VARCHAR(32) NOT NULL,
    alert_type VARCHAR(32) NOT NULL, -- SOS_BUTTON, GAS_LETHAL, WATER_EMPTY, POWER_FAIL
    severity VARCHAR(16) NOT NULL,    -- INFO, WARNING, CRITICAL, EMERGENCY
    description TEXT NOT NULL,
    is_resolved BOOLEAN DEFAULT 0,
    resolved_at TIMESTAMP,
    resolved_by VARCHAR(64),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (node_id) REFERENCES sanitation_nodes(node_id)
);

-- 4. Device Actuation Commands: Log of Servo/Dispense Triggers
CREATE TABLE IF NOT EXISTS actuation_commands (
    command_id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id VARCHAR(32) NOT NULL,
    command_type VARCHAR(32) NOT NULL, -- OPEN_VALVE, CLOSE_VALVE, RESET_ALARM, CALIBRATE
    triggered_by VARCHAR(64) NOT NULL,  -- SENSOR_AUTO, OPERATOR_MANUAL, SYSTEM_TIMER
    executed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(16) DEFAULT 'SUCCESS',
    FOREIGN KEY (node_id) REFERENCES sanitation_nodes(node_id)
);

-- 5. Create Performance Indexes for Fast Time-Series Queries
CREATE INDEX IF NOT EXISTS idx_telemetry_node_time ON telemetry_records (node_id, received_at DESC);
CREATE INDEX IF NOT EXISTS idx_alerts_unresolved ON incident_alerts (is_resolved, created_at DESC);

-- Insert Default Baseline Nodes
INSERT OR IGNORE INTO sanitation_nodes (node_id, location_name, latitude, longitude, status)
VALUES 
    ('NODE_SANITATION_01', 'Shelter Posko A - Zona Evakuasi 1', -6.362141, 106.824964, 'ACTIVE'),
    ('NODE_SANITATION_02', 'Shelter Posko B - Dapur Umum & Medis', -6.363025, 106.825830, 'ACTIVE');
