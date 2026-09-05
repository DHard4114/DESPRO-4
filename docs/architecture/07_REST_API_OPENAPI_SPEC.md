# DOKUMEN SPESIFIKASI REST API (OPENAPI 3.0 / SWAGGER)
## Smart-Sanitation eSOS - Arsitektur Antarmuka Aplikasi

Status Dokumen: SPESIFIKASI API TERKENDALI
Teknologi: OpenAPI 3.0 (Swagger UI) terintegrasi via `swaggo/swag` di Golang
Author: Daffa Hardhan (Manajer Proyek & Penanggung Jawab Full-Stack Server/Dashboard)
QA Tester: Raka Arrayan

---

## 1. Filosofi & Strategi Dokumentasi (Kenapa Swagger?)

Untuk menghindari kelelahan manual (menulis konfigurasi Postman satu per satu) dan memudahkan proses *Quality Assurance* (QA), sistem eSOS mengadopsi standar **OpenAPI 3.0 (Swagger)** dengan pendekatan *Code-First*.

### Alur Otomatisasi (Zero Manual Input):
1. Daffa (Backend) cukup menuliskan *komentar* (Annotation) di atas fungsi *handler* Golang.
2. Library `swaggo/swag` akan membaca komentar tersebut dan secara otomatis merakit file `swagger.json`.
3. Server Go akan menyajikan halaman web interaktif **Swagger UI** di *endpoint* `http://192.168.0.100:8000/swagger/index.html`.
4. Raka (QA) cukup membuka URL tersebut di browser, melihat seluruh daftar API, dan menekan tombol **"Try it out"** untuk menguji API secara langsung (menggantikan Postman).

---

## 2. Struktur Dasar URL (Base URL)
Semua *endpoint* akan dilayani melalui protokol HTTP standar di jaringan lokal:
*   **Base URL Intranet:** `http://192.168.0.100:8000/api/v1`
*   **Format Respons:** Standar industri `application/json`

---

## 3. Daftar Endpoint Utama (API Blueprint)

Berikut adalah *blueprint* fungsi API yang wajib diimplementasikan di Server Go untuk menyuplai data ke layar Dashboard HTML5:

### 3.1. Telemetri & Pemantauan (Telemetry)

#### A. Mengambil Data Kondisi Terkini
*   **Endpoint:** `GET /telemetry/latest`
*   **Fungsi:** Menarik 1 baris data sensor paling *fresh* untuk menggerakkan jarum indikator dan angka teks di *Dashboard*.
*   **Respons Sukses (200 OK):**
    ```json
    {
      "status": "success",
      "data": {
        "node_id": "WC_01",
        "timestamp": "2026-09-06T10:30:00Z",
        "ammonia_ppm": 24.5,
        "h2s_ppm": 2.1,
        "water_percentage": 75.0,
        "aqi_status": "GOOD"
      }
    }
    ```

#### B. Mengambil Data Historis (Untuk Grafik Chart.js)
*   **Endpoint:** `GET /telemetry/history`
*   **Query Params:** `?node=WC_01&range=24h`
*   **Fungsi:** Menarik data hasil *Batch Processing* (Continuous Aggregates) dari TimescaleDB untuk merender grafik deret waktu (*line chart*) 24 jam terakhir.

---

### 3.2. Kontrol Aktuator (Command & Control)

#### A. Memicu Katup Servo Secara Manual (Manual Override)
*   **Endpoint:** `POST /actuator/servo`
*   **Fungsi:** Mengizinkan petugas posko membuka/menutup katup pembuangan gas secara paksa lewat UI Dashboard.
*   **Payload Request:**
    ```json
    {
      "node_id": "WC_01",
      "command": "OPEN_VALVE"
    }
    ```
*   *(Server Go akan menerjemahkan HTTP POST ini menjadi pesan MQTT lalu mengirimkannya ke ESP32 Raka).*

---

### 3.3. Status Kesehatan Sistem (System Health)

#### A. Pengecekan Sistem (Ping)
*   **Endpoint:** `GET /health`
*   **Fungsi:** Dipanggil oleh Dashboard setiap 10 detik untuk memastikan Server Go, koneksi ke PostgreSQL, dan koneksi ke MQTT Mosquitto tidak terputus.
*   **Respons Sukses (200 OK):**
    ```json
    {
      "status": "online",
      "mqtt_broker": "connected",
      "timescaledb": "connected",
      "uptime_seconds": 3600
    }
    ```
