-- =================================================================================
-- SMART-SANITATION eSOS: POSTGRESQL SCHEMA MIGRATION
-- ADR-04: Database PostgreSQL Terintegrasi TimescaleDB
-- ADR-05: Kunci Utama (Primary Key) menggunakan UUIDv7
-- =================================================================================

-- ---------------------------------------------------------------------------------
-- 1. FUNGSI GENERATOR UUIDv7 (OPSI B: PL/pgSQL)
-- ---------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION generate_uuid_v7()
RETURNS uuid
AS $$
DECLARE
    v_time timestamp with time zone := clock_timestamp();
    v_unix_t bigint := extract(epoch from v_time) * 1000;
    v_bytes bytea;
BEGIN
    v_bytes := decode(lpad(to_hex(v_unix_t), 12, '0'), 'hex') || gen_random_bytes(10);
    v_bytes := set_byte(v_bytes, 6, (b'0111' || (get_byte(v_bytes, 6) & b'00001111')::bit(4))::integer);
    v_bytes := set_byte(v_bytes, 8, (b'10' || (get_byte(v_bytes, 8) & b'00111111')::bit(6))::integer);
    RETURN encode(v_bytes, 'hex')::uuid;
END;
$$ LANGUAGE plpgsql VOLATILE;

-- ---------------------------------------------------------------------------------
-- 2. ENUMERASI STATUS (ENUMS)
-- ---------------------------------------------------------------------------------
CREATE TYPE node_status_enum AS ENUM ('ACTIVE', 'INACTIVE', 'MAINTENANCE', 'ALERT_EMERGENCY');
CREATE TYPE aqi_category_enum AS ENUM ('GOOD', 'MODERATE', 'UNHEALTHY', 'HAZARDOUS');
CREATE TYPE alert_status_enum AS ENUM ('OPEN', 'ACKNOWLEDGED', 'RESOLVED', 'FALSE_ALARM');
CREATE TYPE actuation_status_enum AS ENUM ('PENDING', 'TRANSMITTED', 'EXECUTED_SUCCESS', 'EXECUTION_FAILED');

-- ---------------------------------------------------------------------------------
-- 3. TABEL MASTER & KONFIGURASI
-- ---------------------------------------------------------------------------------
CREATE TABLE sanitation_nodes (
    node_id UUID PRIMARY KEY DEFAULT generate_uuid_v7(),
    node_code VARCHAR(32) UNIQUE NOT NULL,
    node_name VARCHAR(128) NOT NULL,
    zone_area VARCHAR(64) NOT NULL,
    latitude DECIMAL(10,7) NOT NULL CHECK (latitude >= -90 AND latitude <= 90),
    longitude DECIMAL(11,7) NOT NULL CHECK (longitude >= -180 AND longitude <= 180),
    status node_status_enum NOT NULL DEFAULT 'ACTIVE',
    last_ping_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE node_threshold_configs (
    config_id UUID PRIMARY KEY DEFAULT generate_uuid_v7(),
    node_id UUID UNIQUE REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    water_critical_low_cm DECIMAL(5,2) DEFAULT 15.00,
    ammonia_warning_ppm DECIMAL(6,2) DEFAULT 25.00,
    ammonia_danger_ppm DECIMAL(6,2) DEFAULT 50.00,
    h2s_warning_ppm DECIMAL(6,2) DEFAULT 10.00,
    h2s_danger_ppm DECIMAL(6,2) DEFAULT 20.00,
    battery_critical_volt DECIMAL(3,2) DEFAULT 3.00,
    updated_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE api_keys (
    key_id UUID PRIMARY KEY DEFAULT generate_uuid_v7(),
    owner_name VARCHAR(64) NOT NULL,
    hashed_key VARCHAR(128) NOT NULL,
    role VARCHAR(32) NOT NULL DEFAULT 'READ_ONLY',
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

-- ---------------------------------------------------------------------------------
-- 4. TABEL TRANSAKSIONAL (TELEMETRI & ALERT)
-- ---------------------------------------------------------------------------------
CREATE TABLE telemetry_records (
    record_id UUID DEFAULT generate_uuid_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
    sequence_no BIGINT NOT NULL,
    water_level_cm DECIMAL(5,2),
    water_volume_percentage DECIMAL(5,2) CHECK (water_volume_percentage >= 0 AND water_volume_percentage <= 100),
    ammonia_ppm DECIMAL(6,2),
    h2s_ppm DECIMAL(6,2),
    air_quality_index aqi_category_enum,
    battery_voltage DECIMAL(4,2),
    solar_charging_active BOOLEAN,
    sos_button_triggered BOOLEAN,
    rssi_dbm INT,
    snr_db DECIMAL(4,1),
    received_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (record_id, received_at)
) PARTITION BY RANGE (received_at);

-- Partisi bulan pertama (Contoh)
CREATE TABLE telemetry_records_2026_09 PARTITION OF telemetry_records
    FOR VALUES FROM ('2026-09-01') TO ('2026-10-01');

CREATE TABLE incident_alerts (
    alert_id UUID PRIMARY KEY DEFAULT generate_uuid_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
    alert_type VARCHAR(32) NOT NULL,
    severity VARCHAR(16) NOT NULL,
    description TEXT,
    status alert_status_enum NOT NULL DEFAULT 'OPEN',
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMPTZ
);

CREATE TABLE actuation_commands (
    command_id UUID PRIMARY KEY DEFAULT generate_uuid_v7(),
    node_id UUID REFERENCES sanitation_nodes(node_id),
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

-- ---------------------------------------------------------------------------------
-- 5. INDEKS & CONSTRAINTS (Kinerja & Integritas)
-- ---------------------------------------------------------------------------------
-- Deduplikasi telemetri LoRa (node_id + sequence_no harus unik per rentang waktu)
CREATE UNIQUE INDEX idx_telemetry_dedup ON telemetry_records (node_id, sequence_no, received_at);

-- Idempotency untuk aktuasi agar tidak ada perintah ganda
CREATE UNIQUE INDEX idx_actuation_idempotency ON actuation_commands (idempotency_key);

-- Indeks untuk pencarian cepat (Dashboard)
CREATE INDEX idx_telemetry_node_received ON telemetry_records (node_id, received_at DESC);
CREATE INDEX idx_telemetry_sos ON telemetry_records (sos_button_triggered) WHERE sos_button_triggered = TRUE;
CREATE INDEX idx_alerts_unresolved ON incident_alerts (status) WHERE status IN ('OPEN', 'ACKNOWLEDGED');

-- ---------------------------------------------------------------------------------
-- 6. TRIGGERS (Notifikasi & Cache Invalidation)
-- ---------------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION notify_threshold_change()
RETURNS trigger AS $$
BEGIN
    PERFORM pg_notify(
        'threshold_config_updated',
        json_build_object(
            'node_id', NEW.node_id,
            'ammonia_danger_ppm', NEW.ammonia_danger_ppm,
            'h2s_danger_ppm', NEW.h2s_danger_ppm,
            'updated_at', NEW.updated_at
        )::text
    );
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_notify_threshold_change
AFTER UPDATE ON node_threshold_configs
FOR EACH ROW
EXECUTE FUNCTION notify_threshold_change();
