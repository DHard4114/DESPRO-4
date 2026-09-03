-- ============================================================================
-- SMART-SANITATION eSOS — ENTERPRISE POSTGRESQL DATABASE SCHEMA (UUID NATIVE)
-- Standard: ACID Compliant (Atomicity, Consistency, Isolation, Durability)
-- Primary Key Strategy: UUIDv4 (gen_random_uuid() RFC 4122) for Multi-Posko Sync
-- Engine: PostgreSQL 15+ / TimescaleDB Compatible
-- Author: Daffa Hardhan (Project Manager & Backend Lead)
-- Project: Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS (DTE FTUI)
-- ============================================================================

-- 0. EXTENSIONS & ENUM TYPES
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

-- Custom Enum Types for Strict Consistency (Type Safety)
DO $$ BEGIN
    CREATE TYPE node_status_enum AS ENUM ('ACTIVE', 'INACTIVE', 'MAINTENANCE', 'ALERT_EMERGENCY');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

DO $$ BEGIN
    CREATE TYPE alert_severity_enum AS ENUM ('INFO', 'WARNING', 'CRITICAL', 'EMERGENCY');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

DO $$ BEGIN
    CREATE TYPE alert_status_enum AS ENUM ('OPEN', 'ACKNOWLEDGED', 'RESOLVED', 'FALSE_ALARM');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

DO $$ BEGIN
    CREATE TYPE command_status_enum AS ENUM ('PENDING', 'TRANSMITTED', 'EXECUTED_SUCCESS', 'EXECUTION_FAILED');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

DO $$ BEGIN
    CREATE TYPE aqi_category_enum AS ENUM ('GOOD', 'MODERATE', 'UNHEALTHY', 'HAZARDOUS');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

-- ============================================================================
-- 1. MASTER TABLE: SANITATION NODES (Fasilitas Sanitasi Posko Bencana)
-- Primary Key: UUIDv4 (gen_random_uuid())
-- ============================================================================
CREATE TABLE IF NOT EXISTS sanitation_nodes (
    node_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    node_code VARCHAR(32) NOT NULL UNIQUE, -- e.g. 'NODE_SANITATION_01'
    node_name VARCHAR(128) NOT NULL,
    zone_area VARCHAR(64) NOT NULL, -- e.g. 'Shelter Posko A - Zona Evakuasi 1'
    latitude DECIMAL(10, 7) NOT NULL CHECK (latitude BETWEEN -90.0 AND 90.0),
    longitude DECIMAL(11, 7) NOT NULL CHECK (longitude BETWEEN -180.0 AND 180.0),
    firmware_version VARCHAR(16) NOT NULL DEFAULT 'v1.0.0',
    hardware_revision VARCHAR(16) NOT NULL DEFAULT 'Rev-2.0',
    lora_frequency_hz BIGINT NOT NULL DEFAULT 433000000,
    status node_status_enum NOT NULL DEFAULT 'ACTIVE',
    installed_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_ping_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 2. CONFIGURATION TABLE: THRESHOLDS PER NODE (Batas Ambang Kritis Sensor)
-- Primary Key: UUIDv4 (gen_random_uuid())
-- ============================================================================
CREATE TABLE IF NOT EXISTS node_threshold_configs (
    config_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    node_id UUID NOT NULL UNIQUE REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    water_tank_height_cm DECIMAL(5, 2) NOT NULL DEFAULT 100.00 CHECK (water_tank_height_cm > 0),
    water_critical_low_cm DECIMAL(5, 2) NOT NULL DEFAULT 15.00 CHECK (water_critical_low_cm >= 0),
    water_warning_low_cm DECIMAL(5, 2) NOT NULL DEFAULT 30.00 CHECK (water_warning_low_cm > water_critical_low_cm),
    ammonia_warning_ppm DECIMAL(6, 2) NOT NULL DEFAULT 25.00 CHECK (ammonia_warning_ppm > 0),
    ammonia_danger_ppm DECIMAL(6, 2) NOT NULL DEFAULT 50.00 CHECK (ammonia_danger_ppm > ammonia_warning_ppm),
    h2s_warning_ppm DECIMAL(6, 2) NOT NULL DEFAULT 10.00 CHECK (h2s_warning_ppm > 0),
    h2s_danger_ppm DECIMAL(6, 2) NOT NULL DEFAULT 20.00 CHECK (h2s_danger_ppm > h2s_warning_ppm),
    battery_critical_volt DECIMAL(3, 2) NOT NULL DEFAULT 3.00 CHECK (battery_critical_volt >= 2.5),
    battery_warning_volt DECIMAL(3, 2) NOT NULL DEFAULT 3.40 CHECK (battery_warning_volt > battery_critical_volt),
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 3. TIME-SERIES TABLE: TELEMETRY RECORDS (Data Deret Waktu Sensor)
-- Primary Key: Composite (record_id UUID, received_at TIMESTAMPTZ) for Range Partitioning
-- ============================================================================
CREATE TABLE IF NOT EXISTS telemetry_records (
    record_id UUID NOT NULL DEFAULT gen_random_uuid(),
    node_id UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    node_code VARCHAR(32) NOT NULL, -- Redundant for lightning-fast querying
    sequence_no BIGINT NOT NULL CHECK (sequence_no >= 0),
    
    -- Sensor Cairan (Ultrasonik Waterproof JSN-SR04T)
    water_level_cm DECIMAL(5, 2) NOT NULL CHECK (water_level_cm BETWEEN 0.0 AND 200.0),
    water_volume_percentage DECIMAL(5, 2) NOT NULL CHECK (water_volume_percentage BETWEEN 0.0 AND 100.0),
    
    -- Sensor Gas Kualitas Udara (MQ-137 Ammonia & MQ-136 H2S)
    ammonia_ppm DECIMAL(6, 2) NOT NULL CHECK (ammonia_ppm >= 0.0),
    h2s_ppm DECIMAL(6, 2) NOT NULL CHECK (h2s_ppm >= 0.0),
    air_quality_index aqi_category_enum NOT NULL DEFAULT 'GOOD',
    
    -- Manajemen Catu Daya Surya (Panel 10Wp + Baterai 1S4P 18650)
    battery_voltage DECIMAL(4, 2) NOT NULL CHECK (battery_voltage BETWEEN 0.0 AND 5.50),
    battery_percentage DECIMAL(5, 2) NOT NULL CHECK (battery_percentage BETWEEN 0.0 AND 100.0),
    solar_charging_active BOOLEAN NOT NULL DEFAULT TRUE,
    
    -- Status Aktuator & Tombol Darurat
    valve_servo_open BOOLEAN NOT NULL DEFAULT FALSE,
    sos_button_triggered BOOLEAN NOT NULL DEFAULT FALSE,
    anomaly_detected BOOLEAN NOT NULL DEFAULT FALSE,
    
    -- Metrik Kualitas Sinyal Nirkabel LoRa RA-02 (SX1278)
    rssi_dbm INT NOT NULL CHECK (rssi_dbm BETWEEN -140 AND 0),
    snr_db DECIMAL(4, 1) NOT NULL,
    
    received_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    -- Composite Primary Key with Timestamp for Partition Compatibility
    PRIMARY KEY (record_id, received_at)
) PARTITION BY RANGE (received_at);

-- Partisi Bulanan (Semester Gasal 2026/2027)
CREATE TABLE IF NOT EXISTS telemetry_records_2026_09 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-09-01 00:00:00+00') TO ('2026-10-01 00:00:00+00');
CREATE TABLE IF NOT EXISTS telemetry_records_2026_10 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-10-01 00:00:00+00') TO ('2026-11-01 00:00:00+00');
CREATE TABLE IF NOT EXISTS telemetry_records_2026_11 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-11-01 00:00:00+00') TO ('2026-12-01 00:00:00+00');
CREATE TABLE IF NOT EXISTS telemetry_records_2026_12 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-12-01 00:00:00+00') TO ('2027-01-01 00:00:00+00');

-- ============================================================================
-- 4. EVENT & ALARM TABLE: INCIDENT ALERTS (Manajemen Peringatan Darurat)
-- Primary Key: UUIDv4 (gen_random_uuid())
-- ============================================================================
CREATE TABLE IF NOT EXISTS incident_alerts (
    alert_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    node_id UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    node_code VARCHAR(32) NOT NULL,
    alert_code VARCHAR(32) NOT NULL, -- 'SOS_BUTTON_TRIGGERED', 'LETHAL_H2S_DETECTED', 'AMMONIA_THRESHOLD_EXCEEDED', 'WATER_SUPPLY_DEPLETED', 'BATTERY_CRITICAL'
    severity alert_severity_enum NOT NULL DEFAULT 'WARNING',
    description TEXT NOT NULL,
    trigger_value DECIMAL(8, 2), -- Nilai sensor saat alert aktif (e.g. 15.5 ppm)
    status alert_status_enum NOT NULL DEFAULT 'OPEN',
    is_resolved BOOLEAN GENERATED ALWAYS AS (status = 'RESOLVED') STORED,
    acknowledged_at TIMESTAMPTZ,
    acknowledged_by VARCHAR(64),
    resolved_at TIMESTAMPTZ,
    resolved_by VARCHAR(64),
    resolution_notes TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 5. AUDIT TRAIL TABLE: ACTUATION COMMANDS (Log Eksekusi Aktuator Servo)
-- Primary Key: UUIDv4 (gen_random_uuid())
-- ============================================================================
CREATE TABLE IF NOT EXISTS actuation_commands (
    command_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    node_id UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    node_code VARCHAR(32) NOT NULL,
    command_type VARCHAR(32) NOT NULL, -- 'OPEN_VALVE_90', 'CLOSE_VALVE_0', 'TRIGGER_BUZZER', 'RESET_SYSTEM'
    target_angle_deg INT CHECK (target_angle_deg BETWEEN 0 AND 180),
    triggered_by VARCHAR(64) NOT NULL, -- 'SENSOR_AUTO_TRIGGER', 'OPERATOR_DASHBOARD', 'EMERGENCY_OVERRIDE'
    operator_id VARCHAR(64),
    status command_status_enum NOT NULL DEFAULT 'PENDING',
    execution_latency_ms INT,
    executed_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 6. SYSTEM AUDIT TABLE: AUDIT LOGS (Integritas & Audit Sistem)
-- Primary Key: UUIDv4 (gen_random_uuid())
-- ============================================================================
CREATE TABLE IF NOT EXISTS system_audit_logs (
    log_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    action_type VARCHAR(64) NOT NULL, -- 'CONFIG_CHANGE', 'DATABASE_BACKUP', 'GATEWAY_DISCONNECT', 'OPERATOR_LOGIN'
    actor_id VARCHAR(64) NOT NULL,
    ip_address VARCHAR(45) NOT NULL,
    details JSONB,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 7. PERFORMANCE INDEXES (Optimasi Kecepatan Query Dashboard)
-- ============================================================================
CREATE INDEX IF NOT EXISTS idx_telemetry_node_received ON telemetry_records (node_code, received_at DESC);
CREATE INDEX IF NOT EXISTS idx_telemetry_sos ON telemetry_records (sos_button_triggered, received_at DESC) WHERE sos_button_triggered = TRUE;
CREATE INDEX IF NOT EXISTS idx_alerts_unresolved ON incident_alerts (status, created_at DESC) WHERE status IN ('OPEN', 'ACKNOWLEDGED');
CREATE INDEX IF NOT EXISTS idx_actuation_pending ON actuation_commands (status, created_at DESC) WHERE status = 'PENDING';
CREATE INDEX IF NOT EXISTS idx_audit_created ON system_audit_logs (created_at DESC);

-- ============================================================================
-- 8. AUTOMATED TRIGGERS FOR TIMESTAMPS (Konsistensi Data Otomatis)
-- ============================================================================
CREATE OR REPLACE FUNCTION update_timestamp_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_update_nodes_timestamp
BEFORE UPDATE ON sanitation_nodes
FOR EACH ROW EXECUTE FUNCTION update_timestamp_column();

CREATE OR REPLACE TRIGGER trg_update_thresholds_timestamp
BEFORE UPDATE ON node_threshold_configs
FOR EACH ROW EXECUTE FUNCTION update_timestamp_column();

CREATE OR REPLACE TRIGGER trg_update_alerts_timestamp
BEFORE UPDATE ON incident_alerts
FOR EACH ROW EXECUTE FUNCTION update_timestamp_column();

-- Update Node Last Ping automatically on new telemetry record
CREATE OR REPLACE FUNCTION update_node_last_ping()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE sanitation_nodes
    SET last_ping_at = NEW.received_at,
        status = CASE 
            WHEN NEW.sos_button_triggered THEN 'ALERT_EMERGENCY'::node_status_enum
            ELSE 'ACTIVE'::node_status_enum
        END
    WHERE node_id = NEW.node_id;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_update_node_ping
AFTER INSERT ON telemetry_records
FOR EACH ROW EXECUTE FUNCTION update_node_last_ping();

-- ============================================================================
-- 9. ANALYTICAL VIEW: HOURLY SUMMARY PER NODE
-- ============================================================================
CREATE OR REPLACE VIEW v_hourly_sanitation_summary AS
SELECT 
    node_code,
    date_trunc('hour', received_at) AS time_bucket,
    COUNT(*) AS total_samples,
    ROUND(AVG(water_level_cm), 2) AS avg_water_level_cm,
    ROUND(MIN(water_level_cm), 2) AS min_water_level_cm,
    ROUND(AVG(ammonia_ppm), 2) AS avg_ammonia_ppm,
    ROUND(MAX(ammonia_ppm), 2) AS max_ammonia_ppm,
    ROUND(AVG(h2s_ppm), 2) AS avg_h2s_ppm,
    ROUND(MAX(h2s_ppm), 2) AS max_h2s_ppm,
    ROUND(AVG(battery_voltage), 2) AS avg_battery_voltage,
    SUM(CASE WHEN sos_button_triggered THEN 1 ELSE 0 END) AS total_sos_triggers
FROM telemetry_records
GROUP BY node_code, date_trunc('hour', received_at)
ORDER BY time_bucket DESC;

-- ============================================================================
-- 10. SEED INITIAL DATA (Data Inisialisasi Posko A & Posko B)
-- ============================================================================
INSERT INTO sanitation_nodes (node_id, node_code, node_name, zone_area, latitude, longitude, status)
VALUES 
    ('a0000000-0000-0000-0000-000000000001', 'NODE_SANITATION_01', 'Unit Sanitasi Mandiri 01', 'Shelter Posko A - Zona Evakuasi 1', -6.3621410, 106.8249640, 'ACTIVE'),
    ('a0000000-0000-0000-0000-000000000002', 'NODE_SANITATION_02', 'Unit Sanitasi Mandiri 02', 'Shelter Posko B - Dapur Umum & Medis', -6.3630250, 106.8258300, 'ACTIVE')
ON CONFLICT (node_code) DO NOTHING;

INSERT INTO node_threshold_configs (config_id, node_id, water_tank_height_cm, water_critical_low_cm, ammonia_warning_ppm, ammonia_danger_ppm, h2s_warning_ppm, h2s_danger_ppm)
VALUES 
    ('b0000000-0000-0000-0000-000000000001', 'a0000000-0000-0000-0000-000000000001', 100.00, 15.00, 25.00, 50.00, 10.00, 20.00),
    ('b0000000-0000-0000-0000-000000000002', 'a0000000-0000-0000-0000-000000000002', 100.00, 15.00, 25.00, 50.00, 10.00, 20.00)
ON CONFLICT (node_id) DO NOTHING;
