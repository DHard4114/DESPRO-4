# DOKUMEN SPESIFIKASI REST API (OPENAPI 3.0 / SWAGGER)
## Smart-Sanitation eSOS - Kontrak End-to-End Backend ke Frontend

Status Dokumen: SPESIFIKASI API TERKENDALI (CONTROLLED BASELINE)  
Teknologi: OpenAPI 3.0 (Swagger UI) terintegrasi via `swaggo/swag` di Golang  
Author: Daffa Hardhan (Manajer Proyek & Penanggung Jawab Full-Stack Server/Dashboard)  
QA Tester: Raka Arrayan  

---

## 1. Filosofi & Strategi Dokumentasi (Kenapa Swagger?)
Untuk menghindari kelelahan manual konfigurasi QA, sistem eSOS mengadopsi standar **OpenAPI 3.0 (Swagger)** dengan pendekatan *Code-First*. Daffa hanya perlu menulis *comment* di Golang, lalu URL `http://192.168.0.100:8000/swagger/index.html` akan memunculkan UI interaktif. Raka tinggal klik tombol **"Try it out"** tanpa perlu Postman.

---

## 2. Kontrak API End-to-End (Sinkronisasi dengan ERD PostgreSQL)

Spesifikasi di bawah ini adalah **Kontrak Final (Data Contract)** antara Server Go dan Web Dashboard. Semua endpoint telah dipetakan $100\%$ secara ketat sesuai relasi tabel di `01_DATABASE_ARCHITECTURE_AND_ERD.md`.

### 2.1. Manajemen Entitas Posko & Node (`SANITATION_NODES`)
Mengelola data induk (Master Data) perangkat keras di lapangan.

*   `GET /api/v1/nodes` -> Mengambil daftar seluruh unit WC (Status, Koordinat, Baterai).
*   `GET /api/v1/nodes/{uuid}` -> Detail spesifik satu unit WC.
*   `POST /api/v1/nodes` -> Mendaftarkan alat ESP32 baru ke jaringan posko.
*   **Contoh Respons JSON (Get Node):**
    ```json
    {
      "node_id": "8f8b8a6a-7b5e...",
      "node_code": "WC_01",
      "zone_area": "Zona Evakuasi A",
      "status": "ACTIVE",
      "battery_percentage": 85.5
    }
    ```

### 2.2. Konfigurasi Ambang Batas Bahaya (`NODE_THRESHOLD_CONFIGS`)
Mengizinkan operator Dashboard menyetel ulang sensitivitas gas dan air tanpa membongkar *coding* ESP32.

*   `GET /api/v1/configs/{node_id}` -> Mengambil aturan ambang batas alat saat ini.
*   `PUT /api/v1/configs/{node_id}` -> Mengubah parameter (Misal: menaikkan batas bahaya gas).
*   **Contoh Payload PUT JSON:**
    ```json
    {
      "water_critical_low_cm": 15.0,
      "ammonia_danger_ppm": 50.0,
      "h2s_danger_ppm": 20.0
    }
    ```

### 2.3. Data Telemetri & Sensor (`TELEMETRY_RECORDS` - TimescaleDB)
Jalur API untuk merender angka grafik dan indikator real-time.

*   `GET /api/v1/telemetry/latest?node_code=WC_01` -> Menarik 1 baris terakhir (Untuk indikator jarum Dashboard).
*   `GET /api/v1/telemetry/history?node_code=WC_01&range=24h` -> Menarik array data dari *Continuous Aggregates* TimescaleDB (Untuk merender grafik Chart.js).
*   **Contoh Respons JSON:**
    ```json
    {
      "timestamp": "2026-09-06T10:30:00Z",
      "water_level_cm": 45.5,
      "water_volume_percentage": 75.0,
      "ammonia_ppm": 24.5,
      "h2s_ppm": 2.1,
      "air_quality_index": "GOOD",
      "solar_charging_active": true
    }
    ```

### 2.4. Manajemen Peringatan Bencana (`INCIDENT_ALERTS`)
Mengelola alarm darurat saat gas melampaui batas atau tombol fisik SOS ditekan pengungsi.

*   `GET /api/v1/alerts?status=UNRESOLVED` -> Menampilkan semua alarm berkedip merah di Dashboard yang belum ditangani.
*   `PUT /api/v1/alerts/{alert_uuid}/resolve` -> Tombol bagi petugas untuk mematikan sirine/alarm setelah situasi dicek.
*   **Contoh Respons JSON:**
    ```json
    {
      "alert_id": "1b9c...",
      "alert_type": "GAS_TOXICITY",
      "severity": "CRITICAL",
      "description": "Bahaya: H2S melebihi 20ppm di WC 01",
      "is_resolved": false
    }
    ```

### 2.5. Kontrol Aktuator Servo (`ACTUATION_COMMANDS`)
Mengontrol perangkat keras pembuka katup ventilasi tangki septic.

*   `POST /api/v1/actuator/override` -> Memicu paksa katup lewat Dashboard. (Server Go akan mengubah REST API ini menjadi pesan MQTT yang ditembakkan ke antena LoRa ESP32).
*   **Contoh Payload JSON:**
    ```json
    {
      "node_code": "WC_01",
      "command": "OPEN_VALVE",
      "operator_notes": "Sirkulasi gas manual"
    }
    ```

### 2.6. Kesehatan Intranet Posko (System Health)
*   `GET /api/v1/health` -> Untuk ikon sinyal di pojok Dashboard. Memeriksa status hidup/mati dari *MQTT Broker*, *PostgreSQL*, dan *Router CPE220*.
