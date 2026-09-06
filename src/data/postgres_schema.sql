-- =================================================================================
-- SMART-SANITATION eSOS: POSTGRESQL SCHEMA
-- ADR-04: Database PostgreSQL Terintegrasi TimescaleDB
-- ADR-06: Seluruh Primary Key UUIDv7 Konsisten (Tidak Ada Campuran v4)
-- ADR-07: Threshold Cache Invalidation via LISTEN/NOTIFY
-- =================================================================================

-- ---------------------------------------------------------------------------------
-- 1. EKSTENSI YANG DIBUTUHKAN
-- ---------------------------------------------------------------------------------
CREATE EXTENSION IF NOT EXISTS pgcrypto;
-- CREATE EXTENSION IF NOT EXISTS pg_uuidv7;  -- Opsi A (jika tersedia)
-- CREATE EXTENSION IF NOT EXISTS timescaledb; -- Aktifkan jika TimescaleDB terinstal

-- ---------------------------------------------------------------------------------
-- 2. FUNGSI GENERATOR UUIDv7 (Opsi B: PL/pgSQL Kustom)
-- ---------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION uuid_generate_v7() RETURNS UUID AS $$
DECLARE
    unix_ts_ms BYTEA;
    uuid_bytes BYTEA;
BEGIN
    unix_ts_ms := substring(int8send(floor(extract(epoch FROM clock_timestamp()) * 1000)::bigint) FROM 3);
    uuid_bytes := unix_ts_ms || gen_random_bytes(10);
    uuid_bytes := set_byte(uuid_bytes, 6, (b'0111' || get_byte(uuid_bytes, 6)::bit(4))::bit(8)::int);
    uuid_bytes := set_byte(uuid_bytes, 8, (b'10' || get_byte(uuid_bytes, 8)::bit(6))::bit(8)::int);
    RETURN encode(uuid_bytes, 'hex')::UUID;
END;
$$ LANGUAGE plpgsql VOLATILE;

-- ---------------------------------------------------------------------------------
-- 3. ENUMERASI STATUS
-- ---------------------------------------------------------------------------------
CREATE TYPE node_status_enum AS ENUM ('ACTIVE', 'INACTIVE', 'MAINTENANCE', 'ALERT_EMERGENCY');
CREATE TYPE aqi_category_enum AS ENUM ('GOOD', 'MODERATE', 'UNHEALTHY', 'HAZARDOUS');
CREATE TYPE alert_status_enum AS ENUM ('OPEN', 'ACKNOWLEDGED', 'RESOLVED', 'FALSE_ALARM');
CREATE TYPE actuation_status_enum AS ENUM ('PENDING', 'TRANSMITTED', 'EXECUTED_SUCCESS', 'EXECUTION_FAILED');

-- ---------------------------------------------------------------------------------
-- 4. TABEL MASTER & KONFIGURASI
-- ---------------------------------------------------------------------------------
CREATE TABLE sanitation_nodes (
    node_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_code VARCHAR(32) UNIQUE NOT NULL,
    node_name VARCHAR(128) NOT NULL,
    zone_area VARCHAR(64) NOT NULL,
    latitude DECIMAL(10,7) NOT NULL CHECK (latitude >= -90 AND latitude <= 90),
    longitude DECIMAL(11,7) NOT NULL CHECK (longitude >= -180 AND longitude <= 180),
    firmware_version VARCHAR(32),
    hardware_revision VARCHAR(32),
    lora_frequency_hz BIGINT,
    status node_status_enum NOT NULL DEFAULT 'ACTIVE',
    installed_at TIMESTAMPTZ,
    last_ping_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE node_threshold_configs (
    config_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id UUID UNIQUE REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    water_tank_height_cm DECIMAL(5,2),
    water_critical_low_cm DECIMAL(5,2) DEFAULT 15.00,
    water_warning_low_cm DECIMAL(5,2) DEFAULT 20.00,
    ammonia_warning_ppm DECIMAL(6,2) DEFAULT 25.00,
    ammonia_danger_ppm DECIMAL(6,2) DEFAULT 50.00,
    h2s_warning_ppm DECIMAL(6,2) DEFAULT 10.00,
    h2s_danger_ppm DECIMAL(6,2) DEFAULT 20.00,
    battery_critical_volt DECIMAL(3,2) DEFAULT 3.00,
    battery_warning_volt DECIMAL(3,2) DEFAULT 3.30,
    updated_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE api_keys (
    key_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
    key_hash VARCHAR(128) NOT NULL,
    scope VARCHAR(32) NOT NULL DEFAULT 'INGEST_ONLY',
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    revoked_at TIMESTAMPTZ
);

-- ---------------------------------------------------------------------------------
-- 5. TABEL TRANSAKSIONAL
-- ---------------------------------------------------------------------------------
CREATE TABLE telemetry_records (
    record_id UUID DEFAULT uuid_generate_v7(),
    received_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    node_id UUID REFERENCES sanitation_nodes(node_id),
    node_code VARCHAR(32),
    sequence_no BIGINT NOT NULL,
    water_level_cm DECIMAL(5,2),
    water_volume_percentage DECIMAL(5,2) CHECK (water_volume_percentage >= 0 AND water_volume_percentage <= 100),
    ammonia_ppm DECIMAL(6,2),
    h2s_ppm DECIMAL(6,2),
    air_quality_index aqi_category_enum,
    battery_voltage DECIMAL(4,2),
    battery_percentage DECIMAL(5,2),
    solar_charging_active BOOLEAN,
    valve_servo_open BOOLEAN,
    sos_button_triggered BOOLEAN,
    anomaly_detected BOOLEAN,
    rssi_dbm INT,
    snr_db DECIMAL(4,1),
    PRIMARY KEY (record_id, received_at)
) PARTITION BY RANGE (received_at);

CREATE TABLE telemetry_records_2026_09 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-09-01') TO ('2026-10-01');

CREATE TABLE incident_alerts (
    alert_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
    node_code VARCHAR(32),
    alert_code VARCHAR(32),
    severity VARCHAR(16) NOT NULL,
    description TEXT,
    trigger_value DECIMAL(8,2),
    status alert_status_enum NOT NULL DEFAULT 'OPEN',
    is_resolved BOOLEAN GENERATED ALWAYS AS (status IN ('RESOLVED', 'FALSE_ALARM')) STORED,
    acknowledged_at TIMESTAMPTZ,
    acknowledged_by VARCHAR(64),
    resolved_at TIMESTAMPTZ,
    resolved_by VARCHAR(64),
    resolution_notes TEXT,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE actuation_commands (
    command_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
    node_code VARCHAR(32),
    idempotency_key VARCHAR(64) NOT NULL,
    command_type VARCHAR(32) NOT NULL,
    target_angle_deg INT CHECK (target_angle_deg >= 0 AND target_angle_deg <= 180),
    triggered_by VARCHAR(32) NOT NULL,
    operator_id VARCHAR(64),
    status actuation_status_enum NOT NULL DEFAULT 'PENDING',
    execution_latency_ms INT,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    executed_at TIMESTAMPTZ
);

CREATE TABLE system_audit_logs (
    log_id UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    action_type VARCHAR(64) NOT NULL,
    actor_id VARCHAR(64),
    ip_address VARCHAR(45),
    trace_id VARCHAR(64),
    details JSONB,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

-- ---------------------------------------------------------------------------------
-- 6. INDEKS & CONSTRAINTS
-- ---------------------------------------------------------------------------------
CREATE UNIQUE INDEX idx_actuation_idempotency ON actuation_commands (idempotency_key);
CREATE INDEX idx_telemetry_dedup ON telemetry_records (node_code, sequence_no);
CREATE INDEX idx_telemetry_node_received ON telemetry_records (node_id, received_at DESC);
CREATE INDEX idx_telemetry_sos ON telemetry_records (sos_button_triggered) WHERE sos_button_triggered = TRUE;
CREATE INDEX idx_alerts_unresolved ON incident_alerts (status) WHERE status IN ('OPEN', 'ACKNOWLEDGED');

-- ---------------------------------------------------------------------------------
-- 7. TRIGGERS (Cache Invalidation — ADR-07)
-- ---------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION notify_threshold_change() RETURNS TRIGGER AS $$
BEGIN
    PERFORM pg_notify('threshold_config_updated', NEW.node_id::text);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_notify_threshold_change
AFTER INSERT OR UPDATE ON node_threshold_configs
FOR EACH ROW EXECUTE FUNCTION notify_threshold_change();
