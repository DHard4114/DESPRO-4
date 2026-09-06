# MASTER IMPLEMENTATION BRIEF & CODING PROTOCOL (v5.0 ULTIMATE)
## Smart-Sanitation eSOS — Panduan Eksekusi Koding Terpadu (Zero-Trust, Hard Real-Time, & Docs-First)

> **CARA PENGGUNAAN (SYSTEM PROMPT):**
> File ini adalah konstitusi mutlak bagi AI Coding Agent atau Programmer Manusia. Dokumen ini mendefinisikan batasan *Hard Real-Time System* (berbasis FreeRTOS Kernel), mitigasi perangkat keras fisik, arsitektur data *End-to-End*, dan Peta Jalan 6 Fase. Jangan memulai penulisan kode sebelum Anda mengasimilasi setiap parameter dalam dokumen ini bersama `docs/architecture/` dan `docs/hardware_references/`.

> **WAJIB lampirkan juga isi lengkap (bukan ringkasan) seluruh file berikut yang sudah final** — jangan andalkan agent untuk "mengingat" isinya dari percakapan sebelumnya:
> - `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md` ⭐ (wajib pertama)
> - `docs/architecture/01_DATABASE_ARCHITECTURE_AND_ERD.md`
> - `docs/architecture/02_BACKEND_ETL_AND_API_ARCHITECTURE.md`
> - `docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md`
> - `docs/architecture/04_GATEWAY_ROUTER_NETWORK_PIPELINE.md`
> - `docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md`
> - `docs/architecture/06_REST_API_OPENAPI_SPEC.md`
> - `docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md`

---

## 0. PROTOKOL WAJIB — ZERO-TRUST & DOCS-FIRST

Pelanggaran terhadap salah satu poin di bawah ini dianggap **kegagalan total tugas**, terlepas seberapa bagus hasil kerja lainnya.

### 0.1 Dilarang Merekonstruksi Dokumen dari Ingatan
Jika Anda mencari sebuah file di `docs/architecture/` (misal via `Get-ChildItem` / `ls`) dan file itu **tidak ditemukan**, Anda **DILARANG KERAS** menulis ulang/mengarang versi Anda sendiri berdasarkan ringkasan percakapan atau asumsi.

**Yang wajib Anda lakukan sebagai gantinya:**
```
STOP. Berhenti total.
Laporkan ke pengguna: "File docs/architecture/{nama_file} tidak ditemukan di repo.
Saya butuh isi lengkap file ini (bukan ringkasan) sebelum melanjutkan, karena file ini
adalah sumber kebenaran mengikat, bukan sesuatu yang boleh saya rekonstruksi sendiri."
```
Tunggu pengguna menyediakan isi file yang sebenarnya. Jangan lanjut dengan versi karangan sendiri "sebagai sementara".

### 0.2 The Great Purge (Mulai dari Nol)
Direktori `src/` adalah kertas putih. DILARANG me-refactor atau menggunakan kode lama (seperti HTTP Ingest, SQLite, atau Arduino Single-Loop). Bangun murni dari awal (*green-field*). **DILARANG** menulis atau mengucapkan kalimat semacam *"sekarang kita migrasi dari HTTP ke MQTT"*, *"upgrade dari SQLite ke PostgreSQL"*, atau kerangka pikir migrasi sistem produksi lain manapun. Ini **BUKAN migrasi**. Berdasarkan ADR-01/ADR-06, kode lama dianggap tidak pernah eksis.

### 0.3 Tidak Ada Asumsi, Hanya Presisi
Jika detail sebuah fungsi/fitur tidak ada di dokumen `docs/`, BERHENTI. Minta penjelasan. Jangan berhalusinasi menyisipkan arsitektur tambahan. Jika suatu detail implementasi tidak dijelaskan, pilih pendekatan paling konsisten dengan pola yang SUDAH ada di dokumen lain, dan **tulis komentar eksplisit** di kode menandai asumsi tersebut beserta ADR yang relevan.

### 0.4 Setiap Perubahan Kode WAJIB Dapat Ditelusuri ke ADR
Setiap commit/PR menyebut ID ADR terkait. Contoh:
```
feat(mqtt): implement subscriber worker pool [ADR-01, ADR-07]
```

### 0.5 Gaya Komunikasi: Faktual, Bukan Sanjungan
Dilarang membuka/menutup laporan dengan pujian berlebihan ("mahakarya", "Super Master", "sekelas Principal Engineer", dsb). Laporkan pekerjaan secara ringkas: apa yang dikerjakan, file apa yang berubah, apa yang masih perlu diverifikasi pengguna. Tidak lebih, tidak kurang.

---

## 1. PRINSIP REKAYASA (ACID, DRY, & DETERMINISM)

### 1.1 End-to-End ACID Compliance
| Properti | Lapisan Firmware (ESP32) | Lapisan Backend (Go) | Lapisan Database (PostgreSQL) |
| :--- | :--- | :--- | :--- |
| **Atomicity** | Transmisi LoRa biner *all-or-nothing*. | Setiap batch ETL dibungkus `BEGIN...COMMIT`. | DML transaksional penuh. |
| **Consistency** | C-Struct `__attribute__((packed))` menjaga tipe data bit-exact. | JSON Envelope dari Gateway di-*unmarshal* ke `struct` Go yang identik. | Tipe kolom `DECIMAL`, `TIMESTAMPTZ`, FK constraints. |
| **Isolation** | FreeRTOS IPC Queues mengisolasi Task antar-Core. | Worker Pool Go memproses data secara memori-terisolasi per Goroutine. | PostgreSQL MVCC (Read Committed). |
| **Durability** | Gateway: pointer `head/tail/count` di-*commit* ke Flash (`meta.dat`). | UUIDv7 di-*commit* ke WAL PostgreSQL. | WAL + fsync. |

### 1.2 RTOS Determinism & Memory Efficiency
| Larangan | Alasan | Pengganti yang Benar |
| :--- | :--- | :--- |
| `delay()` | Memblokir CPU, memboroskan baterai. | `vTaskDelay(pdMS_TO_TICKS(ms))` |
| `malloc()` / `new` / `String` di loop | Heap Fragmentation di SRAM terbatas. | Tipe data statis, `char array`, struct. |
| Operasi berat di ISR | Menghambat Kernel Scheduler. | `xQueueSendFromISR` / `xTaskNotifyFromISR` |
| Variabel global tanpa pelindung | Race Condition antar-Core. | FreeRTOS Queue (`xQueue`) atau Mutex. |

---

## 2. ATURAN MUTLAK IMPLEMENTASI (LOW-LEVEL CONSTRAINTS)

Instruksi teknis mendetail agar tidak ada kesalahan fatal saat koding:

### 2.1 Topologi Jaringan (Open Intranet)
Sistem ini beroperasi di zona *blank spot*. **DILARANG** membuat tabel `API_KEYS` di database atau menyisipkan *Middleware Authorization JWT* di Golang REST API. Seluruh endpoint berjalan dalam mode Open Intranet (Tanpa Autentikasi/Login) sesuai ADR-05.

### 2.2 Kontrak Payload LoRa Biner
Node WC (LoRa Tx) **DILARANG MENGGUNAKAN JSON**. Transmisi radio WAJIB menggunakan C-Struct Binary `__attribute__((packed))` berukuran maksimal 33 bytes (terdefinisi di `docs/hardware_references/01`). ESP32 Gateway-lah yang membaca *raw bytes* ini dan mengubahnya menjadi JSON Envelope menggunakan library `ArduinoJson`.

```cpp
// KONTRAK PAYLOAD BINER MUTLAK (Max 50 Bytes)
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Selalu 1
    char node_code[8];      // Contoh: "WC_01" (Null terminated)
    uint32_t sequence_no;   // Counter pesan
    float water_level_cm;
    float ammonia_ppm;
    float h2s_ppm;
    float battery_voltage;
    uint8_t sos_triggered;  // 1 = True, 0 = False
};
```

### 2.3 Mekanisme Half-Duplex (RX Window)
Karena Node WC hanya punya 1 radio LoRa (SX1278), SETELAH memancarkan data sensor, `vTaskLoRaTx` WAJIB membuka mode Receive selama **2000ms** untuk menangkap paket *Downlink* (Perintah Servo). Mekanisme ini mirip LoRaWAN Class A. Jika timeout, kembali tidur.

### 2.4 Stabilitas Daya (Light-Sleep & Power Locks) — ADR-09
| Langkah | Perintah | Keterangan |
| :--- | :--- | :--- |
| 1. CPU Bangun (Timer/Interrupt) | `esp_pm_lock_acquire(ESP_PM_NO_LIGHT_SLEEP)` | Tahan CPU di Active Mode. |
| 2. Jeda Stabilisasi PLL | `vTaskDelay(pdMS_TO_TICKS(10))` | Tunggu Clock internal stabil. |
| 3. Eksekusi (Baca ADC, Tx LoRa, RX Window) | — | Proses data sensor dan radio. |
| 4. Selesai | `esp_pm_lock_release()` | Izinkan Scheduler menidurkan CPU. |

**DILARANG** menggunakan `Deep-Sleep` (akan merusak RAM dan menghancurkan FreeRTOS Queues). Sistem secara eksklusif menggunakan **Light-Sleep + Tickless Idle Mode** sesuai ADR-09.

### 2.5 Mitigasi Hardware Bawaan
| Komponen | Masalah Fisik | Mitigasi Firmware |
| :--- | :--- | :--- |
| Sensor Ultrasonik `JSN-SR04T` | Blind zone bawaan 20-25 cm. | `if (dist < 25 \|\| dist == 0) dist = 25;` Paksa ke nilai Tangki Kritis/Penuh. |
| Tombol Darurat SOS | Mechanical bouncing memicu interrupt ganda. | Software Debouncing 300ms di ISR via `xTaskGetTickCountFromISR()`. |
| LoRa DIO0 (SX1278) | Radio blocking saat TX/RX. | Pin DIO0 wajib memicu Interrupt (TX Done / RX Done) + Semaphore ke Task. |

### 2.6 DRY & Konfigurasi Terpusat
| Domain | Aturan | Lokasi File |
| :--- | :--- | :--- |
| Firmware | Identitas node (`WC_01`), Pinout GPIO, parameter LoRa **DILARANG** disebar di `.cpp`. | `src/firmware/include/config.h` |
| Go Server | Kredensial Mosquitto (`MQTT_USER`, `MQTT_PASS`), DSN PostgreSQL **DILARANG** di-hardcode. | File `.env` lokal posko. |
| Database | Angka ambang batas alarm **DILARANG** di-hardcode di `.go` atau `.cpp`. | Tabel `node_threshold_configs` + `LISTEN/NOTIFY`. |

---

## 3. TOPOLOGI TASK FreeRTOS & QUEUE (IPC MAP)

### 3.1 Pemetaan Task
| Nama Task | Target Board | Core | Priority | Stack (Bytes) | Deskripsi |
| :--- | :--- | :---: | :---: | :---: | :--- |
| `vTaskSensors` | Node WC | 0 | 1 | 2048 | Baca ADC (Gas, Ultrasonik), rata-rata, push ke Queue. |
| `vTaskActuator` | Node WC | 0 | 2 | 2048 | Putar Servo MG996R via PWM, hanya dari Queue. |
| `vTaskLoRaTx` | Node WC | 1 | 3 | 4096 | Kirim struct biner + buka RX Window 2000ms (Class A). |
| `vTaskLoRaRx` | Gateway | 1 | 3 | 4096 | Tangkap paket biner LoRa secara kontinu. |
| `vTaskMqttTx` | Gateway | 0 | 2 | 4096 | Deserialize biner → JSON, Publish ke Mosquitto. |

### 3.2 Topologi Queue
| Queue Handle | Board | Produsen | Konsumen | Catatan |
| :--- | :--- | :--- | :--- | :--- |
| `xQueueSensorData` | Node WC | `vTaskSensors` | `vTaskLoRaTx` | Data sensor. ISR SOS juga push via `xQueueSendFromISR`. |
| `xQueueCommand` | Node WC | `vTaskLoRaTx` (RX Window) | `vTaskActuator` | Perintah buka/tutup katup dari Dashboard. |
| `xQueueTelemetry` | Gateway | `vTaskLoRaRx` | `vTaskMqttTx` | Jika Wi-Fi mati → data dibelokkan ke LittleFS. |

### 3.3 ISR & Scheduler
| Sumber Interrupt | Pin | Aksi ISR (Max ~10μs) | Task yang Dibangunkan |
| :--- | :--- | :--- | :--- |
| Tombol SOS (EXTI) | `PIN_BTN_SOS` (GPIO 27) | `xQueueSendFromISR(xQueueSensorData, &emergencyPayload)` + Debounce 300ms. | `vTaskLoRaTx` |
| LoRa DIO0 TX Done | `PIN_LORA_DIO0` (GPIO 2) | `xSemaphoreGiveFromISR(semTxDone)` | `vTaskLoRaTx` (lanjut ke RX Window). |
| LoRa DIO0 RX Done | `PIN_LORA_DIO0` (GPIO 2) | `xSemaphoreGiveFromISR(semRxDone)` | `vTaskLoRaRx` (Gateway) / `vTaskLoRaTx` (Node, RX Window). |

---

## 4. ARSITEKTUR STORE-AND-FORWARD (GATEWAY RESILIENSI)

### 4.1 LittleFS Circular Buffer
| Parameter | Nilai | Keterangan |
| :--- | :--- | :--- |
| Tipe | Fixed-Size Binary Circular Buffer | BUKAN append file teks. |
| File Data | `buffer.dat` | Berisi slot-slot berukuran `sizeof(TelemetryPayload)`. |
| File Metadata | `meta.dat` | Menyimpan `head`, `tail`, `count` secara persisten. |
| Kapasitas | 500 pesan | ~16.5 KB (33 bytes × 500). |
| Trigger Flush | Wi-Fi reconnect berhasil | `vTaskMqttTx` membaca buffer FIFO hingga kosong. |

### 4.2 Siklus Operasi Gateway
```
1. vTaskLoRaRx menangkap paket biner dari udara.
2. Push ke xQueueTelemetry.
3. vTaskMqttTx mencoba Publish ke Mosquitto.
   ├─ SUKSES → Lanjut ke paket berikutnya.
   └─ GAGAL (Wi-Fi mati) →
       ├─ Tulis paket ke slot buffer.dat[tail].
       ├─ Update meta.dat (tail++, count++).
       └─ Saat Wi-Fi pulih → Flush FIFO dari buffer.dat[head] hingga kosong.
```

---

## 5. MANAJEMEN BUILD (PlatformIO)

### 5.1 Pemisahan Lingkungan Build
Firmware Node dan Gateway berada dalam **satu repositori** (sharing `config.h` dan `TelemetryPayload`), namun **WAJIB dipisah secara ketat**:

```ini
[env:node_wc]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
build_flags = -D IS_NODE_WC
lib_deps =
    jgromes/RadioLib
    teckel12/NewPing
    # ⛔ DILARANG KERAS: WiFi.h, PubSubClient, ArduinoJson

[env:gateway_router]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
build_flags = -D IS_GATEWAY
lib_deps =
    jgromes/RadioLib
    knolleary/PubSubClient
    bblanchon/ArduinoJson
```

### 5.2 Seleksi Kompilasi via Preprocessor
```cpp
// Di dalam main.cpp:
#ifdef IS_NODE_WC
    // Init Sensors, LoRa Tx, Actuator, SOS ISR
#elif defined(IS_GATEWAY)
    // Init LoRa Rx, Wi-Fi, MQTT, LittleFS
#else
    #error "Build flag IS_NODE_WC atau IS_GATEWAY wajib didefinisikan!"
#endif
```

---

## 6. PETA JALAN EKSEKUSI 6 FASE (PHASED ROLLOUT)

Kerjakan implementasi blok demi blok secara berurutan. **Jangan melompat ke fase berikutnya** sebelum pengguna memberi instruksi "Lanjut ke Fase X".

### FASE 0: Database & Backend Foundations
| Deliverable | Detail |
| :--- | :--- |
| `src/server/go.mod` | Dependensi: `pgx/v5`, `paho.mqtt.golang`, `google/uuid` (UUIDv7), `swaggo/swag`, `godotenv`. |
| `src/data/postgres_schema.sql` | DDL PostgreSQL: UUIDv7 custom function, semua tabel sesuai ERD (`01_DATABASE`), indeks B-Tree, Trigger `LISTEN/NOTIFY` untuk threshold cache. **TANPA tabel `API_KEYS`**. |
| Verifikasi | `psql -f postgres_schema.sql` berhasil tanpa error. |

### FASE 1: The Ingestion Engine (MQTT & ETL)
| Deliverable | Detail |
| :--- | :--- |
| MQTT Subscriber | `paho.mqtt.golang` terhubung `clean_session=false`. Kredensial dibaca dari `.env` (`MQTT_USER`, `MQTT_PASS`). |
| Worker Pool | N Goroutine (default 8) mengonsumsi `chan MQTTMessage` (kapasitas 1000). |
| Deduplikasi | LRU cache `(node_code → last_sequence_no)`. Buang pesan duplikat dari QoS 1 retry. |
| Threshold Cache | `sync.RWMutex` + `LISTEN/NOTIFY` live-refresh dari PostgreSQL. |
| Micro-batch | Kapasitas 50 rekaman / flush timer 3 detik. `pgx.CopyFrom` untuk bulk insert. |

### FASE 2: API REST v1 & WebSocket Hub
| Deliverable | Detail |
| :--- | :--- |
| Router | `gorilla/mux` atau `chi`. Base path: `/api/v1/`. |
| Auth | **TIDAK ADA.** Open Intranet. |
| Idempotency | `Idempotency-Key` header untuk `POST /actuator/commands`. |
| Error Envelope | `{"error": {"code": "...", "message": "..."}, "trace_id": "..."}` |
| WebSocket Hub | Fan-out `sync.Mutex`. Events: `TELEMETRY_STREAM`, `EMERGENCY_ALERT`, `GATEWAY_STATUS`, `ACTUATOR_STATUS`. |
| Swagger | Auto-generate via `swaggo/swag`. |

### FASE 3: Firmware Node WC (LoRa Tx, Sensor, ISR)
| Deliverable | Detail |
| :--- | :--- |
| `platformio.ini` | `[env:node_wc]`. **DILARANG** me-load library Wi-Fi/MQTT. |
| `config.h` | Struct Biner packed, GPIO, parameter LoRa, identitas Node. |
| `vTaskSensors` | Baca ADC, Moving Average (10 sampel), push ke `xQueueSensorData`. Filter Blind Zone ultrasonik. |
| `vTaskLoRaTx` | Transmit struct biner + RX Window 2000ms (Class A). DIO0 Interrupt + Semaphore. |
| `vTaskActuator` | Putar Servo MG996R via PWM. Hanya membaca dari `xQueueCommand`. |
| ISR SOS | EXTI pada `PIN_BTN_SOS`. Debouncing 300ms. `xQueueSendFromISR`. |
| Power | Light-Sleep + Tickless Idle + Power Locks (`esp_pm_lock_t`). Jeda 10ms setelah wake-up. |

### FASE 4: Firmware Gateway (LoRa Rx, MQTT Wi-Fi, Resiliensi)
| Deliverable | Detail |
| :--- | :--- |
| `platformio.ini` | `[env:gateway_router]`. Termasuk PubSubClient dan ArduinoJson. |
| `vTaskLoRaRx` | Tangkap raw bytes, push ke `xQueueTelemetry`. |
| `vTaskMqttTx` | Deserialize struct → JSON. Publish ke Mosquitto. Jika gagal → tulis ke LittleFS circular buffer. |
| `vTaskMqttRx` | Subscribe `esos/posko-a/+/command`. Teruskan perintah aktuator ke `xQueueCommand` → LoRa Downlink. |
| LittleFS | `buffer.dat` (Fixed-Size Circular) + `meta.dat` (head, tail, count persisten). Flush saat Wi-Fi pulih. |

### FASE 5: Web Dashboard Frontend (UI)
| Deliverable | Detail |
| :--- | :--- |
| `index.html` | Single-page. Tailwind CSS + Chart.js. |
| WebSocket Client | Native `new WebSocket()`. Auto-Reconnect setiap 3 detik. |
| Panel Kontrol | Tombol buka/tutup katup → `POST /api/v1/actuator/commands`. Status async via WS event `ACTUATOR_STATUS`. |
| Grafik | Grafik real-time level air, amonia, H2S, baterai. |

---

## 7. LARANGAN EKSPLISIT (THE "NEVER DO THIS" LIST)

| # | Larangan | ADR Terkait |
| :---: | :--- | :---: |
| 1 | ❌ **DILARANG** melakukan *hardcode* angka ambang batas (threshold alarm) di file `.go` atau `.cpp`. Ambil via Database/LISTEN. | ADR-07 |
| 2 | ❌ **DILARANG** mematikan ESP32 ke mode `Deep-Sleep`. | ADR-09 |
| 3 | ❌ **DILARANG** membuat tabel `API_KEYS` atau menyisipkan Autentikasi (JWT/Login) di REST API. | ADR-05 |
| 4 | ❌ **DILARANG** memutar aktuator Servo langsung dari ISR atau langsung dari Task LoRa. Kirim perintah melalui Queue. | ADR-01 |
| 5 | ❌ **DILARANG** mencampur UUIDv4 murni (`gen_random_uuid()`) dengan skema database UUIDv7. | ADR-06 |
| 6 | ❌ **DILARANG** membiarkan Node WC memiliki kode apapun yang mengaktifkan radio Wi-Fi. | ADR-02 |
| 7 | ❌ **DILARANG** menggunakan `delay()` blocking di firmware manapun. | — |
| 8 | ❌ **DILARANG** menggunakan `malloc()`/`new`/`String` di dalam loop RTOS. | — |
| 9 | ❌ **DILARANG** membuat endpoint REST baru di luar kontrak `06_REST_API_OPENAPI_SPEC.md` tanpa mengusulkan ADR. | ADR-05 |
| 10 | ❌ **DILARANG** menambahkan jalur ingest HTTP langsung sebagai *default path*. HTTP hanya *fallback-only*. | ADR-01 |

---

*Dokumen ini adalah sumber kebenaran operasional. Jika ada konflik antara dokumen ini dengan kode, dokumen ini yang benar.*
