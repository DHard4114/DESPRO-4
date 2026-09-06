# DOKUMEN SPESIFIKASI REST API (OPENAPI 3.0 / SWAGGER)
## Smart-Sanitation eSOS — Kontrak End-to-End Backend ke Frontend

Status Dokumen: **SPESIFIKASI API TERKENDALI (CONTROLLED BASELINE) — v2.0 MATURED**
Mengacu pada: `00_ARCHITECTURE_DECISION_RECORD.md` (ADR-05)
Teknologi: OpenAPI 3.0 (Swagger UI) terintegrasi via `swaggo/swag` di Golang
Author: Daffa Hardhan

---

## 1. Filosofi & Strategi Dokumentasi

Sistem eSOS mengadopsi standar **OpenAPI 3.0 (Swagger)** dengan pendekatan *Code-First*. Daffa menulis *annotation comment* di Golang, lalu URL `http://192.168.0.100:8000/swagger/index.html` memunculkan UI interaktif untuk QA.

---

## 2. Konvensi Umum

*   **Base URL:** `http://192.168.0.100:8000/api/v1`
*   **Format Respons:** `application/json`
*   **Autentikasi:** Mode Open Intranet (Tanpa Autentikasi/Login). API dapat diakses publik di jaringan lokal posko darurat.

---

## 3. Error Envelope Standar

Semua respons gagal menggunakan format envelope yang konsisten:

```json
{
  "status": "error",
  "error": {
    "code": "RESOURCE_NOT_FOUND",
    "message": "Node dengan ID yang diminta tidak ditemukan.",
    "trace_id": "019abc12-..."
  }
}
```

`trace_id` berformat UUIDv7 dan dilog ke tabel `SYSTEM_AUDIT_LOGS` untuk korelasi debugging.

---

## 4. Kontrak Endpoint End-to-End (Sinkronisasi dengan ERD PostgreSQL)

### 4.1. Manajemen Node (`SANITATION_NODES`)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `GET` | `/nodes` | Daftar seluruh unit WC | `200` |
| `GET` | `/nodes/{node_id}` | Detail satu unit WC | `200`, `404` |
| `POST` | `/nodes` | Registrasi ESP32 baru | `201`, `409` |

### 4.2. Konfigurasi Ambang Batas (`NODE_THRESHOLD_CONFIGS`)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `GET` | `/nodes/{node_id}/config` | Ambil threshold aktif | `200`, `404` |
| `PUT` | `/nodes/{node_id}/config` | Ubah threshold (trigger NOTIFY) | `200`, `400` |

### 4.3. Data Telemetri (`TELEMETRY_RECORDS`)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `GET` | `/nodes/{node_id}/telemetry/latest` | 1 baris terbaru (indikator Dashboard) | `200`, `404` |
| `GET` | `/nodes/{node_id}/telemetry/history?range=24h&cursor=...` | Cursor-based pagination data historis | `200` |

> **Catatan Pagination:** `GET /telemetry/history` menggunakan *cursor-based pagination* (`cursor` = `record_id` UUIDv7 terakhir yang dilihat klien). Ukuran halaman default 100, maksimal 1000.

### 4.4. Peringatan Insiden (`INCIDENT_ALERTS`)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `GET` | `/alerts?status=OPEN` | Daftar alarm aktif | `200` |
| `GET` | `/alerts/{alert_id}` | Detail satu alarm | `200`, `404` |
| `PUT` | `/alerts/{alert_id}/acknowledge` | Petugas memverifikasi alarm | `200` |
| `PUT` | `/alerts/{alert_id}/resolve` | Petugas menyelesaikan insiden | `200` |

### 4.5. Perintah Aktuator (`ACTUATION_COMMANDS`)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `POST` | `/actuator/commands` | Kirim perintah servo (asinkron) | `202`, `409` |

Header wajib: `Idempotency-Key: <uuid>`. Server menyimpan hasil request pertama; request kedua dengan key sama mengembalikan hasil identik tanpa eksekusi ulang.

Respons `202 Accepted`:
```json
{
  "status": "accepted",
  "data": {
    "command_id": "019abc34-...",
    "status": "PENDING"
  }
}
```

Konfirmasi final diterima klien via WebSocket event `ACTUATOR_STATUS`.

### 4.6. Kesehatan Sistem (System Health)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `GET` | `/health` | Status ringkas (`UP` / `DEGRADED`) | `200` |
| `GET` | `/health/detailed` | Status per komponen | `200` |

```json
{
  "status": "UP",
  "components": {
    "mqtt_broker": "connected",
    "timescaledb": "connected",
    "uptime_seconds": 3600
  }
}
```

### 4.7. WebSocket Events (Push dari Server ke Dashboard)

Koneksi: `ws://192.168.0.100:8000/ws`

| Event Type | Sumber | Deskripsi |
|:---|:---|:---|
| `TELEMETRY_STREAM` | MQTT telemetry topic | Data sensor terbaru |
| `EMERGENCY_ALERT` | Threshold engine | Alarm gas/SOS/air kritis |
| `GATEWAY_STATUS` | MQTT LWT | Status online/offline Gateway |
| `ACTUATOR_STATUS` | MQTT command/ack | Konfirmasi eksekusi servo |

### 4.8. Telemetry Ingest Fallback (Testing Only)

| Method | Path | Deskripsi | Status Code |
|:---|:---|:---|:---:|
| `POST` | `/telemetry/ingest` | *Fallback-only* untuk testing tanpa hardware | `201` |

> **Peringatan:** Endpoint ini **bukan jalur ingest resmi** (ADR-01). Jalur resmi adalah MQTT. Endpoint ini hanya diaktifkan dalam mode development/testing dan akan me-*publish* data yang diterima ke Mosquitto Broker secara internal sebelum diolah oleh Subscriber normal.

---

## 5. Ringkasan HTTP Status Code

| Code | Makna | Dipakai Pada |
|:---|:---|:---|
| `200` | Sukses | GET, PUT |
| `201` | Berhasil dibuat | POST /nodes, POST /telemetry/ingest |
| `202` | Diterima, diproses asinkron | POST /actuator/commands |
| `400` | Validasi gagal | PUT config dengan nilai di luar rentang |
| `| `404` | Resource tidak ditemukan | GET by ID |
| `409` | Konflik (duplikat idempotency key / node_code) | POST /nodes, POST /actuator |
| `429` | Rate limit terlampaui | Semua endpoint |
| `500` | Internal server error | Semua endpoint |


---

## 6. Pemenuhan Kepatuhan ACID pada Lapis REST API

Seluruh interaksi API mematuhi prinsip ACID untuk mencegah korupsi data akibat koneksi klien yang terputus di tengah jalan (misal: petugas menekan tombol servo tapi kehilangan sinyal Wi-Fi):

1. **Atomicity (Keutuhan):** Endpoint POST dan PUT dibungkus dalam satu blok transaksi database (BEGIN...COMMIT). Pembuatan *Actuation Command* tidak akan tersimpan jika validasi payload gagal di tengah jalan.
2. **Consistency (Konsistensi):** Idempotency-Key menjamin keamanan *retry*. Jika klien menekan tombol "Buka Katup" berulang kali akibat sinyal lag, server akan mengidentifikasinya sebagai satu perintah tunggal (menolak duplikasi).
3. **Isolation (Isolasi):** *Rate Limiting* per API Key diisolasi secara ketat sehingga lonjakan request dari satu klien tidak memblokir antrean request klien lain pada *Router* HTTP Go.
4. **Durability (Ketahanan):** *Error Envelope* yang dikembalikan ke klien mencantumkan 	race_id UUIDv7 yang telah **selesai** ditulis (*flushed*) secara permanen ke dalam tabel SYSTEM_AUDIT_LOGS pada media penyimpanan *TimescaleDB* sebelum HTTP 500 dikirimkan kembali ke peramban.


