# DOKUMEN 00: ARCHITECTURE DECISION RECORD (ADR)
## Smart-Sanitation eSOS - Sumber Kebenaran Tunggal (Single Source of Truth)

Dokumen ini adalah **Architecture Decision Record (ADR)** utama yang mencatat seluruh keputusan arsitektural krusial yang telah disepakati oleh tim. Dokumen ini bertindak sebagai payung hukum teknis tertinggi. Jika ada konflik antara kode sumber dan dokumen lain, **ADR ini yang menjadi acuan final**.

---

### ADR-01: Arsitektur Komunikasi Data Berbasis MQTT
*   **Konteks:** Penggunaan HTTP konvensional membebani daya sensor dan tidak dirancang untuk aliran data IoT berlatensi rendah yang konstan.
*   **Keputusan:** Jalur ingest resmi Gateway→Server menggunakan **MQTT (Eclipse Mosquitto)**, bukan HTTP POST langsung. ESP32 Gateway menerima sinyal radio LoRa lalu meneruskannya ke Mosquitto via Wi-Fi Intranet. Go Server berlangganan (*subscribe*) ke Mosquitto.
*   **Konsekuensi:** Beban jaringan Wi-Fi turun drastis, latensi menjadi sub-milidetik, tetapi membutuhkan *setup* Mosquitto di mesin server.

### ADR-02: Topologi Jaringan Sensor "Star" (Node LoRa-Only)
*   **Konteks:** Sistem membutuhkan konektivitas jarak jauh yang stabil di area tanpa sinyal (blank spot) tanpa membebani daya sensor.
*   **Keputusan:** Sistem mengadopsi **Star Topology** menggunakan modul **LoRa RA-02 433MHz**. ESP32 Node WC bersifat **LoRa-only** (radio Wi-Fi dimatikan total); hanya ESP32 Gateway yang memiliki radio Wi-Fi.
*   **Konsekuensi:** Firmware Node WC murni hanya radio LoRa (tanpa Wi-Fi, menghemat baterai), logika jauh lebih sederhana.

### ADR-03: Resiliensi Jaringan via Store-and-Forward di Level Gateway
*   **Konteks:** Koneksi Wi-Fi Gateway ke CPE220 bisa terputus sewaktu-waktu akibat kondisi pasca-bencana.
*   **Keputusan:** Resiliensi jaringan ditangani via **store-and-forward ring buffer** (kapasitas 500 pesan) di level ESP32 Gateway, bukan dual-path di level Node.
*   **Konsekuensi:** Node tetap *stateless* terhadap kondisi jaringan. Gateway menjadi satu-satunya titik yang menangani kompleksitas *reconnect* dan *buffering*.

### ADR-04: Firmware ESP32 Wajib FreeRTOS Multi-Task
*   **Konteks:** Penggunaan `delay()` di C++ konvensional membuat sensor macet saat mengirim data radio.
*   **Keputusan:** Seluruh firmware ESP32 (Node dan Gateway) wajib menggunakan **FreeRTOS multi-task, non-blocking**, dengan pemetaan task/priority/core eksplisit sesuai `docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md`.
*   **Konsekuensi:** Sensor, radio LoRa, dan aktuator berjalan di *Task* terpisah tanpa saling memblokir. Tidak ada satupun `delay()` blocking di source code.

### ADR-05: Kontrak REST API Enterprise-Grade (Versioned `/api/v1`)
*   **Konteks:** Pengujian manual via Postman melelahkan dan rentan miskomunikasi antara Backend dan Frontend/QA.
*   **Keputusan:** Kontrak REST API mengikuti konvensi enterprise IoT: **versioned path `/api/v1`**, autentikasi **API Key + JWT**, **idempotency key**, **cursor pagination**, dan **error envelope standar**.
*   **Konsekuensi:** Swagger UI (`swaggo/swag`) ter-generate otomatis. QA cukup membuka URL dan klik "Try it out".

### ADR-06: Primary Key UUIDv7 Konsisten di Seluruh Tabel
*   **Konteks:** `Auto-Increment ID` berbahaya untuk sinkronisasi multi-posko. UUIDv4 murni acak menyebabkan *B-Tree index fragmentation*.
*   **Keputusan:** Menggunakan **UUIDv7 (RFC 9562, Time-Ordered)** sebagai Primary Key di **semua** tabel secara konsisten. Tidak ada campuran UUIDv4.
*   **Konsekuensi:** Indeks B-Tree optimal, timestamp pembuatan bisa diekstrak dari UUID tanpa kolom terpisah.

### ADR-07: Ambang Batas Alarm dari Cache, Bukan Hardcode
*   **Konteks:** Ambang batas gas/air yang di-hardcode di kode Go/C++ membuat perubahan konfigurasi membutuhkan recompile dan redeploy.
*   **Keputusan:** Ambang batas alarm dibaca dari **in-memory cache** (`map[node_id]Config` + `sync.RWMutex`) yang di-refresh real-time via **PostgreSQL LISTEN/NOTIFY** (`pg_notify('threshold_config_updated', ...)`).
*   **Konsekuensi:** Operator posko bisa mengubah batas bahaya dari Dashboard tanpa restart server atau reflash firmware.

### ADR-08: Path Dokumentasi Resmi
*   **Konteks:** Dokumentasi arsitektur pernah tersebar di beberapa lokasi (`src/server/docs/`, root, dll.) menyebabkan kebingungan.
*   **Keputusan:** Path dokumentasi resmi adalah `docs/architecture/` dan `docs/hardware_references/` (root-level). Tidak ada dokumen arsitektur di `src/server/docs/` atau lokasi lain.
*   **Konsekuensi:** Seluruh referensi antar-dokumen konsisten menunjuk ke `docs/architecture/` sebagai *single source of truth*.
