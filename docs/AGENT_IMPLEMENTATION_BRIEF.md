# AGENT IMPLEMENTATION BRIEF
## Smart-Sanitation eSOS — Instruksi Eksekusi untuk AI Coding Agent

> **Cara Pakai:** Tempel seluruh isi file ini sebagai instruksi awal (system/task prompt) ke AI coding agent (Claude Code, atau sejenis) di root repositori `DESPRO2_SMART_SANITATION_MODULAR/`. Lampirkan juga dokumen spesifik yang sedang dikerjakan jika agent tidak punya akses baca langsung ke `docs/architecture/`.

---

## 1. PERAN & KONTEKS ANDA

Anda adalah *engineering agent* yang bertugas menyelaraskan **seluruh kode dan dokumen** proyek Smart-Sanitation eSOS agar 100% konsisten dengan `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md` (ADR) — dokumen ini adalah **sumber kebenaran tunggal (single source of truth)**.

**Aturan mutlak yang tidak boleh dilanggar:**
1. **Docs-first, bukan code-first.** Jika ada pertentangan antara kode yang ada di `src/` dengan dokumen di `docs/architecture/`, **dokumen yang benar**. Kode lama dianggap draf usang (*legacy scaffold*) — Anda BOLEH menulis ulang total, bukan sekadar *patch* tambal sulam.
2. **Baca SEMUA dokumen berikut secara berurutan SEBELUM menulis satu baris kode pun:**
   - `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md` ⭐ (wajib pertama)
   - `docs/architecture/01_DATABASE_ARCHITECTURE_AND_ERD.md`
   - `docs/architecture/02_BACKEND_ETL_AND_API_ARCHITECTURE.md`
   - `docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md`
   - `docs/architecture/04_GATEWAY_ROUTER_NETWORK_PIPELINE.md`
   - `docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md`
   - `docs/architecture/06_REST_API_OPENAPI_SPEC.md`
   - `docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md`
3. **Jangan pernah menebak spesifikasi yang tidak ada di dokumen.** Jika suatu detail implementasi tidak dijelaskan (misal: format field baru, nama fungsi), pilih pendekatan paling konsisten dengan pola yang SUDAH ada di dokumen lain, dan **tulis komentar eksplisit** di kode menandai asumsi tersebut beserta ADR yang relevan — jangan diam-diam menyimpang.
4. **Setiap perubahan kode WAJIB dapat ditelusuri ke ADR spesifik.** Setiap commit/PR menyebut ID ADR terkait (contoh: `feat(mqtt): implement subscriber worker pool [ADR-01, ADR-07]`).
5. **Jangan menghapus fungsi yang sudah benar** hanya karena strukturnya berbeda dari dokumen — cek dulu apakah perbedaannya substantif (melanggar ADR) atau kosmetik (variasi penamaan yang tidak bertentangan).

---

## 2. DAFTAR TUGAS PER FASE (Kerjakan Berurutan)

### FASE 0 — Penuntasan Dokumentasi (Prasyarat)
- [ ] **Revisi `docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md`** agar selaras dengan:
  - Event WebSocket baru: `GATEWAY_STATUS` (dipicu LWT MQTT, ADR-01/03) dan `ACTUATOR_STATUS` (dipicu `command/ack`).
  - Alur kontrol servo: tombol Dashboard sekarang memanggil `POST /api/v1/actuator/commands` (respons `202 Accepted`, asinkron) — BUKAN lagi mengubah `innerText` badge secara lokal tanpa efek nyata.
  - Simulator pipeline internal: perbaiki narasi "kirim ke FastAPI/SQLite" menjadi "publish MQTT via Mosquitto → Go Subscriber" (atau jelaskan sebagai *fallback-only* `POST /api/v1/telemetry/ingest` jika tetap dipertahankan untuk testing tanpa hardware).
  - Header autentikasi: tambahkan `Authorization: Bearer <jwt>` pada seluruh contoh `fetch()` ke endpoint `/api/v1/*`.
  - **Definition of Done:** Tidak ada satupun contoh kode/diagram di dokumen ini yang menyebut FastAPI, endpoint `/api/telemetry` (gaya lama tanpa versioning), atau kontrol aktuator tanpa efek backend nyata.

### FASE 1 — Basis Data (ADR-06, ADR-07)
- [ ] Implementasikan `postgres_schema.sql` final: seluruh PK pakai UUIDv7 konsisten (pilih Opsi A `pg_uuidv7` ATAU Opsi B fungsi PL/pgSQL kustom dari `01_DATABASE_ARCHITECTURE_AND_ERD.md` §2.4 — pilih satu, jangan campur).
- [ ] Tambahkan tabel `api_keys` sesuai ERD revisi.
- [ ] Implementasikan trigger `notify_threshold_change()` + `pg_notify` sesuai §5.
- [ ] Tambahkan indeks `idx_actuation_idempotency` (UNIQUE) dan `idx_telemetry_dedup`.
- [ ] **Definition of Done:** `psql` migration berjalan bersih dari kosong; setiap PK yang di-generate terverifikasi berformat UUIDv7 (byte versi = `7`).

### FASE 2 — Backend Go: MQTT & ETL (ADR-01, ADR-07)
- [ ] Ganti seluruh jalur ingest HTTP-only dengan **MQTT Subscriber** (`github.com/eclipse/paho.mqtt.golang`), *persistent session*, *auto-reconnect*.
- [ ] Implementasikan **Worker Pool** (N goroutine dikonfigurasi via env `ETL_WORKER_COUNT`) yang mengonsumsi *buffered channel* dari callback MQTT.
- [ ] Implementasikan **deduplikasi** berbasis `(node_code, sequence_no)`.
- [ ] Implementasikan **Threshold Cache** in-memory + goroutine `LISTEN threshold_config_updated`.
- [ ] Implementasikan publish downlink ke topik `.../command` saat `POST /api/v1/actuator/commands` diterima.
- [ ] **Hapus** logika threshold hardcoded (`ammonia_ppm > 25.0` dst. langsung di kode) — ganti baca dari cache.
- [ ] **Definition of Done:** Simulasi `mosquitto_pub` (lihat `04_GATEWAY_ROUTER_NETWORK_PIPELINE.md` §6) berhasil diterima, diproses, dan tersimpan ke database dengan `record_id` UUIDv7 valid.

### FASE 3 — Backend Go: REST API v1 (ADR-05)
- [ ] Implementasikan seluruh endpoint di `06_REST_API_OPENAPI_SPEC.md` §4 (nodes, config, telemetry, alerts, actuator/commands, health).
- [ ] Implementasikan middleware: API Key validation, JWT validation + role check, `Idempotency-Key` handling (simpan hasil request pertama, kembalikan hasil sama untuk key yang sama), rate limiting.
- [ ] Implementasikan *error envelope* standar (§3) untuk SEMUA respons gagal — tidak ada `http.Error()` plain-text lagi.
- [ ] Implementasikan *cursor-based pagination* untuk `telemetry/history`.
- [ ] **Definition of Done:** Swagger UI (`swaggo/swag`) ter-generate dan menampilkan seluruh endpoint sesuai kontrak; setiap endpoint diuji dengan `curl`/Postman mengembalikan status code sesuai §5.

### FASE 4 — Firmware ESP32 Node WC (ADR-02, ADR-04)
- [ ] Refactor total dari `setup()/loop()` menjadi **FreeRTOS multi-task** sesuai tabel `01_SENSOR_AND_ACTUATOR_REFERENCES.md` §1 (task, priority, core affinity, stack size persis seperti tabel).
- [ ] Migrasi library LoRa dari `sandeepmistry/LoRa` ke **RadioLib** (interrupt-driven, non-blocking).
- [ ] Pastikan **radio Wi-Fi dimatikan total** (`WiFi.mode(WIFI_OFF)`) — Node tidak boleh punya kode apapun yang mengaktifkan Wi-Fi.
- [ ] Implementasikan `vTaskSOSButton` sebagai ISR + `xTaskNotifyFromISR` (bukan polling `digitalRead` di loop biasa).
- [ ] Implementasikan `vTaskWatchdog` dengan `esp_task_wdt`.
- [ ] **Definition of Done:** Firmware compile bersih di PlatformIO; uji serial monitor menunjukkan seluruh task berjalan paralel tanpa satupun `delay()` blocking di source code.

### FASE 5 — Firmware ESP32 Gateway (ADR-01, ADR-02, ADR-03, ADR-04)
- [ ] Buat proyek PlatformIO **terpisah** (`src/firmware_gateway/`) — jangan digabung dengan firmware Node.
- [ ] Implementasikan task sesuai `01_SENSOR_AND_ACTUATOR_REFERENCES.md` §2: `vTaskLoRaListener`, `vTaskMqttPublisher`, `vTaskMqttSubscriber`, `vTaskLoRaDownlinkTx`, `vTaskWiFiSupervisor`, `vTaskHeartbeat`, `vTaskWatchdog`.
- [ ] Implementasikan **ring buffer store-and-forward** (kapasitas 500, FIFO eviction) persis struktur `MqttMessage` di §2.1.
- [ ] Implementasikan **LWT** saat `PubSubClient::connect()` sesuai payload di `05_LORA_MQTT_TELEMETRY_PIPELINE.md` §3.
- [ ] **Definition of Done:** Simulasi mematikan Wi-Fi Gateway selama >1 menit tidak menyebabkan data LoRa yang masuk selama itu hilang — seluruhnya ter-*flush* saat Wi-Fi pulih.

### FASE 6 — Infrastruktur MQTT (ADR-01)
- [ ] Buat `src/config/mosquitto.conf` dan `src/config/mosquitto_acl` persis sesuai `05_LORA_MQTT_TELEMETRY_PIPELINE.md` §7.
- [ ] Buat `src/config/mosquitto_passwd` (via `mosquitto_passwd` tool, JANGAN commit password plaintext — commit instruksi generate saja).
- [ ] *(Opsional tapi direkomendasikan)* Buat `docker-compose.yml` untuk menjalankan Mosquitto + PostgreSQL/TimescaleDB secara lokal untuk kebutuhan development tanpa instalasi manual.
- [ ] **Definition of Done:** `mosquitto -c src/config/mosquitto.conf -v` berjalan tanpa error, menolak koneksi anonymous, ACL per-user berfungsi (uji dengan `mosquitto_pub`/`mosquitto_sub` dari user berbeda).

### FASE 7 — Dashboard Frontend (Menyusul Fase 0)
- [ ] Update `src/server/static/index.html` mengikuti hasil revisi Fase 0.
- [ ] Tambahkan handler WebSocket untuk `GATEWAY_STATUS` dan `ACTUATOR_STATUS`.
- [ ] Ubah `triggerValve()` menjadi `async function` yang benar-benar memanggil `POST /api/v1/actuator/commands` dengan `Idempotency-Key` (bisa pakai `crypto.randomUUID()`), lalu *polling* singkat atau dengarkan `ACTUATOR_STATUS` untuk update UI final.
- [ ] **Definition of Done:** Menekan tombol "Buka Katup" di UI benar-benar mengirim request REST API dan status UI berubah HANYA setelah menerima konfirmasi dari backend — tidak ada lagi perubahan UI yang murni kosmetik tanpa efek nyata.

---

## 3. VERIFIKASI AKHIR (Jalankan Sebelum Melapor Selesai)

```powershell
# 1. Broker MQTT hidup & menolak anonymous
mosquitto -c src/config/mosquitto.conf -v

# 2. Server Go hidup & terhubung ke Broker + DB
.\src\bin\esos-server.exe

# 3. Health check harus UP untuk semua komponen
curl http://localhost:8000/api/v1/health/detailed

# 4. Simulasi data sensor via MQTT (bukan HTTP)
mosquitto_pub -h localhost -p 1883 -u gateway_posko_a -P "<password>" `
  -t "esos/posko-a/WC_01/telemetry" -q 1 `
  -m '{"schema_version":"1.0","node_code":"WC_01","sequence_no":1,"sent_at":"2026-09-06T10:30:00+07:00","payload_type":"TELEMETRY","data":{"water_level_cm":65.0,"ammonia_ppm":8.0,"h2s_ppm":3.0,"battery_voltage":3.95,"sos_button_triggered":false,"rssi_dbm":-70,"snr_db":9.0}}'

# 5. Verifikasi data masuk & UUIDv7 valid
curl http://localhost:8000/api/v1/nodes/{node_id}/telemetry/latest -H "Authorization: Bearer <jwt>"

# 6. Uji idempotency: kirim POST actuator command 2x dengan Idempotency-Key sama, harus hasil identik bukan eksekusi ganda
```

**Setelah seluruh verifikasi lolos**, perbarui:
- `MANIFEST.json` → pastikan `sourceCode[]` mencerminkan struktur file final.
- `05_EVIDENCE_REGISTER.md` → tandai status setiap `EV-W2-*` terkait sebagai selesai dengan link commit/file aktual.
- Laporkan ke pengguna: dokumen mana yang masih punya *gap* implementasi (jika ada) dan ADR mana yang butuh keputusan susulan.

---

## 4. LARANGAN EKSPLISIT

- ❌ Jangan menambahkan jalur ingest HTTP langsung sebagai *default path* — HTTP hanya boleh sebagai *fallback-only endpoint* yang eksplisit ditandai demikian.
- ❌ Jangan biarkan Node WC memiliki kode apapun yang mengaktifkan radio Wi-Fi.
- ❌ Jangan hardcode ambang batas alarm di file `.go`/`.cpp` manapun.
- ❌ Jangan campur UUIDv4 dan UUIDv7 dalam skema database yang sama.
- ❌ Jangan buat endpoint REST baru di luar kontrak `06_REST_API_OPENAPI_SPEC.md` tanpa mengusulkan penambahan ADR terlebih dahulu.
- ❌ Jangan gunakan `delay()` blocking di firmware manapun.
