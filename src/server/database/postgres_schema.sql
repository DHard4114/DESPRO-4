-- ============================================================================
-- Smart-Sanitation eSOS — PostgreSQL Schema (Phase 0)
-- Kepatuhan: ADR-01, ADR-05, ADR-06, ADR-07
-- ============================================================================
-- PRASYARAT: CREATE EXTENSION IF NOT EXISTS pgcrypto;
-- ============================================================================

CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- ============================================================================
-- 1. FUNGSI KUSTOM UUIDv7 (Time-Ordered, RFC 9562)
-- ============================================================================
-- PostgreSQL native gen_random_uuid() menghasilkan UUIDv4 (acak murni).
-- UUIDv7 menyisipkan Unix timestamp (ms) di 48-bit pertama sehingga
-- menghasilkan indeks B-Tree yang optimal (monotonically increasing).
-- ============================================================================

CREATE OR REPLACE FUNCTION uuid_generate_v7()
RETURNS uuid AS $$
DECLARE
    unix_ts_ms  bigint;
    uuid_bytes  bytea;
BEGIN
    unix_ts_ms := (EXTRACT(EPOCH FROM clock_timestamp()) * 1000)::bigint;

    -- 16 random bytes sebagai basis
    uuid_bytes := gen_random_bytes(16);

    -- Byte 0-5: Unix timestamp milliseconds (48-bit, big-endian)
    uuid_bytes := set_byte(uuid_bytes, 0, (unix_ts_ms >> 40)::int & 255);
    uuid_bytes := set_byte(uuid_bytes, 1, (unix_ts_ms >> 32)::int & 255);
    uuid_bytes := set_byte(uuid_bytes, 2, (unix_ts_ms >> 24)::int & 255);
    uuid_bytes := set_byte(uuid_bytes, 3, (unix_ts_ms >> 16)::int & 255);
    uuid_bytes := set_byte(uuid_bytes, 4, (unix_ts_ms >>  8)::int & 255);
    uuid_bytes := set_byte(uuid_bytes, 5, (unix_ts_ms >>  0)::int & 255);

    -- Byte 6: version = 7 (0111xxxx)
    uuid_bytes := set_byte(uuid_bytes, 6, (get_byte(uuid_bytes, 6) & 15) | 112);

    -- Byte 8: variant = 2 (10xxxxxx)
    uuid_bytes := set_byte(uuid_bytes, 8, (get_byte(uuid_bytes, 8) & 63) | 128);

    RETURN encode(uuid_bytes, 'hex')::uuid;
END;
$$ LANGUAGE plpgsql VOLATILE;

-- ============================================================================
-- 2. TABEL: sanitation_nodes
-- ============================================================================

CREATE TABLE IF NOT EXISTS sanitation_nodes (
    node_id             UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_code           VARCHAR(16) NOT NULL UNIQUE,
    node_label          VARCHAR(64) NOT NULL,
    location_lat        DECIMAL(9,6),
    location_lon        DECIMAL(9,6),
    hardware_revision   VARCHAR(16) DEFAULT 'v1.0',
    is_active           BOOLEAN NOT NULL DEFAULT TRUE,
    registered_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_seen_at        TIMESTAMPTZ
);

COMMENT ON TABLE sanitation_nodes IS 'Registry perangkat ESP32 Node WC yang terdaftar di sistem.';

-- ============================================================================
-- 3. TABEL: node_threshold_configs (ADR-07: Threshold dari DB, bukan hardcode)
-- ============================================================================

CREATE TABLE IF NOT EXISTS node_threshold_configs (
    config_id           UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id             UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    metric_name         VARCHAR(32) NOT NULL,
    warning_value       DECIMAL(10,2) NOT NULL,
    critical_value      DECIMAL(10,2) NOT NULL,
    unit                VARCHAR(16) NOT NULL,
    updated_at          TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (node_id, metric_name)
);

COMMENT ON TABLE node_threshold_configs IS 'Konfigurasi ambang batas alarm per sensor per node. Diambil oleh Go Server via LISTEN/NOTIFY cache.';

-- ============================================================================
-- 4. TABEL: telemetry_records (Time-Series, kandidat TimescaleDB Hypertable)
-- ============================================================================

CREATE TABLE IF NOT EXISTS telemetry_records (
    record_id           UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id             UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    node_code           VARCHAR(16) NOT NULL,
    sequence_no         INTEGER NOT NULL,
    water_level_cm      DECIMAL(6,2),
    ammonia_ppm         DECIMAL(8,2),
    h2s_ppm             DECIMAL(8,2),
    battery_voltage     DECIMAL(4,2),
    sos_triggered       BOOLEAN NOT NULL DEFAULT FALSE,
    received_at         TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    processed_at        TIMESTAMPTZ
);

COMMENT ON TABLE telemetry_records IS 'Rekaman telemetri sensor dari ESP32 Node WC via MQTT.';

-- ============================================================================
-- 5. TABEL: incident_alerts
-- ============================================================================

CREATE TABLE IF NOT EXISTS incident_alerts (
    alert_id            UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id             UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    record_id           UUID REFERENCES telemetry_records(record_id),
    alert_type          VARCHAR(32) NOT NULL CHECK (alert_type IN (
                            'AMMONIA_WARNING', 'AMMONIA_CRITICAL',
                            'H2S_WARNING', 'H2S_CRITICAL',
                            'WATER_LEVEL_WARNING', 'WATER_LEVEL_CRITICAL',
                            'SOS_BUTTON', 'BATTERY_LOW', 'NODE_OFFLINE'
                        )),
    severity            VARCHAR(16) NOT NULL CHECK (severity IN ('WARNING', 'CRITICAL', 'EMERGENCY')),
    message             TEXT,
    is_resolved         BOOLEAN NOT NULL DEFAULT FALSE,
    triggered_at        TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    resolved_at         TIMESTAMPTZ
);

COMMENT ON TABLE incident_alerts IS 'Log peringatan insiden yang dibangkitkan oleh mesin ETL berdasarkan threshold.';

-- ============================================================================
-- 6. TABEL: actuation_commands
-- ============================================================================

CREATE TABLE IF NOT EXISTS actuation_commands (
    command_id          UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    node_id             UUID NOT NULL REFERENCES sanitation_nodes(node_id) ON DELETE CASCADE,
    idempotency_key     UUID NOT NULL UNIQUE,
    command_type        VARCHAR(32) NOT NULL CHECK (command_type IN (
                            'OPEN_VALVE', 'CLOSE_VALVE', 'FLUSH_TANK'
                        )),
    status              VARCHAR(16) NOT NULL DEFAULT 'PENDING' CHECK (status IN (
                            'PENDING', 'SENT', 'EXECUTED_SUCCESS', 'EXECUTED_FAIL', 'TIMEOUT'
                        )),
    issued_at           TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    acknowledged_at     TIMESTAMPTZ
);

COMMENT ON TABLE actuation_commands IS 'Perintah kontrol aktuator (Servo MG996R) dari Dashboard ke Node WC via MQTT Downlink.';

-- ============================================================================
-- 7. TABEL: system_audit_logs
-- ============================================================================

CREATE TABLE IF NOT EXISTS system_audit_logs (
    log_id              UUID PRIMARY KEY DEFAULT uuid_generate_v7(),
    trace_id            UUID,
    log_level           VARCHAR(8) NOT NULL CHECK (log_level IN (
                            'DEBUG', 'INFO', 'WARN', 'ERROR', 'FATAL'
                        )),
    source_service      VARCHAR(32) NOT NULL,
    message             TEXT NOT NULL,
    metadata            JSONB,
    created_at          TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

COMMENT ON TABLE system_audit_logs IS 'Jejak audit sistem untuk korelasi error envelope REST API (trace_id).';

-- ============================================================================
-- 8. INDEKS PERFORMA
-- ============================================================================

-- Deduplikasi MQTT: cegah insert ganda dari QoS 1 retry [ADR-01]
CREATE UNIQUE INDEX IF NOT EXISTS idx_telemetry_dedup
    ON telemetry_records (node_code, sequence_no);

-- Idempotency Key untuk perintah aktuator [ADR-05]
CREATE UNIQUE INDEX IF NOT EXISTS idx_actuation_idempotency
    ON actuation_commands (idempotency_key);

-- Query performa: telemetri terbaru per node
CREATE INDEX IF NOT EXISTS idx_telemetry_node_received
    ON telemetry_records (node_id, received_at DESC);

-- Query performa: alert yang belum resolved
CREATE INDEX IF NOT EXISTS idx_alerts_unresolved
    ON incident_alerts (is_resolved, triggered_at DESC)
    WHERE is_resolved = FALSE;

-- Query performa: SOS events
CREATE INDEX IF NOT EXISTS idx_telemetry_sos
    ON telemetry_records (sos_triggered, received_at DESC)
    WHERE sos_triggered = TRUE;

-- ============================================================================
-- 9. TRIGGER: LISTEN/NOTIFY THRESHOLD CACHE INVALIDATION [ADR-07]
-- ============================================================================

CREATE OR REPLACE FUNCTION fn_notify_threshold_change()
RETURNS trigger AS $$
BEGIN
    PERFORM pg_notify(
        'threshold_config_updated',
        json_build_object(
            'operation', TG_OP,
            'node_id',   COALESCE(NEW.node_id, OLD.node_id)::text,
            'metric',    COALESCE(NEW.metric_name, OLD.metric_name)
        )::text
    );
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER trg_threshold_config_changed
    AFTER INSERT OR UPDATE OR DELETE ON node_threshold_configs
    FOR EACH ROW
    EXECUTE FUNCTION fn_notify_threshold_change();

-- ============================================================================
-- 10. TimescaleDB HYPERTABLE (Opsional — uncomment jika ekstensi tersedia)
-- ============================================================================
-- SELECT create_hypertable('telemetry_records', 'received_at',
--     chunk_time_interval => INTERVAL '1 day',
--     if_not_exists => TRUE
-- );

-- ============================================================================
-- SELESAI. Jalankan: psql -U postgres -d esos_db -f postgres_schema.sql
-- ============================================================================
