# Skema Basis Data & Model Relasional: Smart-Sanitation eSOS 🗄️

Direktori ini berisi skrip DDL (*Data Definition Language*) PostgreSQL untuk sistem Smart-Sanitation eSOS, yang dirancang sesuai standar kepatuhan **ADR-01, ADR-05, ADR-06, dan ADR-07**.

---

## 🏛️ Desain Arsitektur Data

1. **UUIDv7 (RFC 9562) Monotonically Increasing Primary Key:**
   Menggunakan fungsi kustom `uuid_generate_v7()` yang menyematkan Unix timestamp (milidetik) pada 48-bit pertama. Ini memberikan keunggulan performa indeks B-Tree yang setara dengan `BIGSERIAL`, namun tetap mempertahankan keunikan global tanpa risiko tabrakan kunci ID terdistribusi.
2. **ACID Compliance & Integritas Referensial Penuh:**
   Semua relasi antar node bilik sanitasi, histori telemetri, log insiden alarm, dan audit jejak perintah aktuasi dijaga ketat menggunakan batasan *foreign key* dan *check constraints*.
3. **Mekanisme Real-Time Sinkronisasi (LISTEN / NOTIFY):**
   Pembaruan batas ambang batas (*threshold*) pada tabel `threshold_configs` memicu fungsi pemicu PostgreSQL `NOTIFY threshold_updated`, yang seketika ditangkap oleh cache in-memory Go Server tanpa perlunya *polling* berulang.

---

## 📊 Entity-Relationship Diagram (ERD)

```mermaid
erDiagram
    sanitation_nodes ||--o{ telemetry_history : "records"
    sanitation_nodes ||--o{ alarm_incidents : "triggers"
    sanitation_nodes ||--o{ actuator_commands : "receives"
    sanitation_nodes ||--o{ threshold_configs : "calibrated_by"

    sanitation_nodes {
        UUID node_id PK
        VARCHAR node_code UK
        VARCHAR node_label
        DECIMAL location_lat
        DECIMAL location_lon
        BOOLEAN is_active
        TIMESTAMPTZ registered_at
        TIMESTAMPTZ last_seen_at
    }

    telemetry_history {
        UUID record_id PK
        UUID node_id FK
        BIGINT sequence_no
        TIMESTAMPTZ recorded_at
        DECIMAL water_level_cm
        DECIMAL ammonia_ppm
        DECIMAL h2s_ppm
        DECIMAL battery_voltage
        BOOLEAN sos_triggered
        TIMESTAMPTZ ingested_at
    }

    alarm_incidents {
        UUID incident_id PK
        UUID node_id FK
        VARCHAR alarm_type
        VARCHAR severity
        TEXT trigger_reason
        TIMESTAMPTZ triggered_at
        BOOLEAN is_resolved
        TIMESTAMPTZ resolved_at
    }

    actuator_commands {
        UUID command_id PK
        UUID node_id FK
        UUID idempotency_key UK
        VARCHAR command_type
        VARCHAR target_state
        VARCHAR status
        TIMESTAMPTZ issued_at
        TIMESTAMPTZ acked_at
    }

    threshold_configs {
        UUID config_id PK
        UUID node_id FK
        DECIMAL max_water_level_cm
        DECIMAL max_ammonia_ppm
        DECIMAL max_h2s_ppm
        DECIMAL min_battery_voltage
        TIMESTAMPTZ updated_at
    }
```

---

## 🚀 Panduan Eksekusi Skema

Untuk menginisialisasi basis data PostgreSQL lokal:

```powershell
# Buat database jika belum ada
psql -U postgres -c "CREATE DATABASE esos_db;"

# Eksekusi DDL schema
psql -U postgres -d esos_db -f src/data/postgres_schema.sql
```
