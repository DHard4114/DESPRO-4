# Smart-Sanitation eSOS - Go Server & Dashboard Engine 🌐

Direktori ini berisi kode sumber mesin server backend berkinerja tinggi yang ditulis menggunakan **Go (Golang)** dan dashboard pemantau darurat (*Emergency Management System / EMS*) berbasis web tunggal (*Single-File Dashboard*).

---

## 🏗️ Diagram Arsitektur Backend (Lambda Architecture)

```mermaid
flowchart TD
    subgraph INGESTION ["1. Ingestion Layer"]
        GW["ESP32 Gateway (WiFi)"] -->|JSON Telemetry| MQTT["Mosquitto MQTT Broker\n(Port 1883)"]
        MQTT -->|Subscribe: esos/+/telemetry| SUB["mqtt/subscriber.go\n(Paho MQTT Client)"]
        SUB -->|Channel: IngestChan| PIPE["etl/pipeline.go\n(Worker Pool)"]
    end

    subgraph ETL_POOL ["2. Concurrent ETL & Processing (8 Goroutines)"]
        PIPE --> W1["Worker 1"]
        PIPE --> W2["Worker 2"]
        PIPE --> Wn["Worker 8..."]
        W1 & W2 & Wn --> CACHE{"In-Memory\nThreshold Cache\n(Atomic/RWMutex)"}
        CACHE -->|Exceeds Threshold?| ALARM["Trigger Alarm Flag\n(SOS / Level Hazard)"]
    end

    subgraph STORAGE ["3. Persistent Storage (PostgreSQL)"]
        W1 & W2 & Wn -->|Batch Channel| BATCH["Dynamic Batch Inserter\n(50 items / 100ms)"]
        BATCH -->|pgxpool Batch Copy| DB[("PostgreSQL 15+\n(UUIDv7, Partitioned)")]
        DB -.->|LISTEN / NOTIFY| CACHE
    end

    subgraph REALTIME ["4. Realtime Broadcast & REST API"]
        PIPE -->|Telemetry Stream| WS_HUB["api/websocket.go\n(Native WebSocket Hub)"]
        WS_HUB -->|JSON Push| WEB["static/index.html\n(Tailwind CSS + Chart.js)"]
        
        WEB -->|POST /api/v1/actuators/lock\n(Header: X-Idempotency-Key)| API_H["api/handlers.go\n(Chi Router)"]
        API_H -->|Publish Downlink Cmd| MQTT
    end

    classDef ing fill:#e0f2fe,stroke:#0284c7,stroke-width:2px;
    classDef etl fill:#fef3c7,stroke:#d97706,stroke-width:2px;
    classDef db fill:#dcfce7,stroke:#16a34a,stroke-width:2px;
    classDef rt fill:#f3e8ff,stroke:#9333ea,stroke-width:2px;

    class GW,MQTT,SUB,PIPE ing;
    class W1,W2,Wn,CACHE,ALARM etl;
    class BATCH,DB db;
    class WS_HUB,WEB,API_H rt;
```

---

## 📂 Struktur Modul Server

| Direktori / Berkas | Fungsi & Tanggung Jawab Teknis |
| :--- | :--- |
| [`main.go`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/main.go) | Titik masuk utama aplikasi, inisialisasi modul, dan mekanisme *graceful shutdown* (10 detik). |
| [`api/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/api) | Routing REST HTTP (Chi router), controller endpoint telemetri/aktuator, dan WebSocket Hub. |
| [`config/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/config) | Parser konfigurasi berbasis variabel lingkungan (`.env`). |
| [`database/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/database) | Manajemen koneksi pool PostgreSQL (`pgxpool`) dan migrasi schema DDL. |
| [`etl/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/etl) | Worker pool konkuren (8 workers), buffer batch inserter, dan cache in-memory dengan sinkronisasi `LISTEN/NOTIFY`. |
| [`models/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/models) | Definisi *struct* data untuk telemetri sensor, perintah aktuasi, log alarm, dan payload WebSocket. |
| [`mqtt/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/mqtt) | Klien MQTT subscriber (Paho) untuk menerima telemetri gateway dan publisher downlink perintah bilik. |
| [`static/`](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/src/server/static) | Berkas web frontend mandiri (`index.html`) dengan visualisasi grafik *real-time* via Chart.js. |

---

## 🔒 Fitur Keamanan & Reliabilitas

1. **Idempotency Key pada Perintah Aktuator:**
   Setiap instruksi penguncian pintu bilik WC via REST API wajib menyertakan *header* `X-Idempotency-Key` (UUIDv4). Hal ini mencegah aktuasi ganda (*duplicate locking*) akibat *network retry* dari operator dashboard.
2. **Dynamic Batch Inserter:**
   Alih-alih melakukan eksekusi SQL `INSERT` satu per satu untuk setiap data LoRa, server mengelompokkan payload dalam jendela 100 ms atau 50 rekaman ke dalam saluran *batch* untuk efisiensi I/O disk PostgreSQL yang optimal.
3. **In-Memory Threshold Synchronization:**
   Batas peringatan level septik tank dan konsentrasi gas disimpan dalam RAM untuk pencocokan cepat tanpa membebani database. Jika operator mengubah ambang batas di database/UI, PostgreSQL mengirim sinyal `NOTIFY threshold_updated` yang seketika memperbarui cache RAM server.

---

## 🚀 Panduan Menjalankan

1. **Pastikan PostgreSQL & Mosquitto Berjalan:**
   ```powershell
   # Mosquitto
   mosquitto -v
   ```
2. **Konfigurasi Berkas `.env`:**
   ```env
   DB_URL=postgres://postgres:password@localhost:5432/esos_db?sslmode=disable
   MQTT_BROKER=tcp://localhost:1883
   HTTP_PORT=8000
   ```
3. **Kompilasi & Jalankan Server:**
   ```powershell
   cd src/server
   go run main.go
   ```
4. **Akses Dashboard:**
   Buka peramban web ke `http://localhost:8000`.
