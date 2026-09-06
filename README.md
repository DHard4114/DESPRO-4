# REPOSITORI UTAMA PROYEK — SMART-SANITATION eSOS
## Sistem Pemantauan Sanitasi Cerdas Berbasis IoT & Transmisi Nirkabel Daerah Bencana

Status Dokumen: REPOSITORI MASTER TERKENDALI (CONTROLLED BASELINE) — **v3.0 Docs-First Architecture**
Institusi: Departemen Teknik Elektro, Fakultas Teknik Universitas Indonesia (DTE FTUI)
Mata Kuliah: Desain Proyek 2 (Semester Gasal 2026/2027)
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
Kelompok: 4 (Empat)

> **Prinsip Docs-First:** Seluruh implementasi kode dalam repositori ini mengacu secara mengikat ke `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md` sebagai sumber kebenaran tunggal. Jika ada perbedaan antara kode di `src/` dengan dokumen di `docs/architecture/`, **dokumen yang berlaku**, bukan kode — kode akan disesuaikan mengikuti dokumen pada iterasi pengembangan berikutnya.

---

## 1. Glosarium Singkatan & Istilah Resmi Repositori (Master Glossary)

| Singkatan | Kepanjangan Lengkap | Penjelasan Sederhana & Fungsi dalam Sistem eSOS |
|:---|:---|:---|
| **eSOS** | *Emergency Sanitation Operating System* | Nama sistem perangkat lunak dan arsitektur pemantauan fasilitas sanitasi darurat terpadu kelompok 4 Despro. |
| **WBS** | *Work Breakdown Structure* | Dekomposisi hierarkis 14 pekan seluruh ruang lingkup pekerjaan proyek. |
| **BoM** | *Bill of Materials* | Daftar lengkap 15 komponen perangkat keras, spesifikasi, RAB, dan status verifikasi vendor. |
| **RACI** | *Responsible, Accountable, Consulted, Informed* | Matriks pembagian peran dan tanggung jawab kerja 205 jam. |
| **ETL** | *Extract, Transform, Load* | Alur pemrosesan data: ekstraksi pesan MQTT, transformasi kalibrasi gas/air, pemuatan ke basis data. |
| **ACID** | *Atomicity, Consistency, Isolation, Durability* | Standar keandalan transaksi basis data. |
| **UUIDv7** | *Universally Unique Identifier Version 7 (RFC 9562)* | Kunci unik global 128-bit terurut waktu, dipakai konsisten di seluruh tabel (lihat ADR-06). |
| **MQTT** | *Message Queuing Telemetry Transport* | Protokol *publish-subscribe* resmi untuk seluruh jalur ingest data Gateway→Server (ADR-01). |
| **Mosquitto** | *Eclipse Mosquitto MQTT Broker* | Broker MQTT lokal yang wajib berjalan sebagai prasyarat sebelum Go Server dapat menerima data. |
| **FreeRTOS** | *Free Real-Time Operating System* | Kernel RTOS yang mendasari seluruh firmware ESP32 (Node & Gateway) — multi-task, non-blocking (ADR-04). |
| **ERD** | *Entity Relationship Diagram* | Diagram visual relasi antar-tabel basis data. |
| **REST v1** | *REST API Version 1* | Kontrak API enterprise: `/api/v1/...`, autentikasi API Key/JWT, idempotency key, cursor pagination (ADR-05). |
| **ADR** | *Architecture Decision Record* | Dokumen pemersatu keputusan arsitektur final — `docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md`. |
| **LoRa** | *Long Range Radio* | Modulasi radio 433 MHz jarak jauh; satu-satunya radio yang dimiliki Node WC (ADR-02). |
| **IP54** | *Ingress Protection 54* | Standar ketahanan casing terhadap debu dan percikan air. |

---

## 2. Ringkasan Eksekutif Sistem Smart-Sanitation eSOS

**Smart-Sanitation eSOS** adalah sistem rekayasa sanitasi cerdas modular untuk fasilitas tanggap darurat pasca-bencana di wilayah tanpa jangkauan seluler (*blank spot*). Sistem ini mengintegrasikan:

1. **Penginderaan Lingkungan Otomatis:** Sensor gas MQ-137 ($NH_3$), MQ-136 ($H_2S$), sensor jarak ultrasonik JSN-SR04T.
2. **Kendali Aktuasi Higienis:** ESP32 DevKitC V4 + servo MG996R untuk katup nirsentuh, dieksekusi via perintah downlink MQTT.
3. **Komunikasi Nirkabel Star Topology:** Node WC **LoRa-only** (tanpa Wi-Fi) → ESP32 Gateway (LoRa RX + Wi-Fi TX) → Mosquitto Broker via TP-Link CPE220. Lihat `docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md`.
4. **Kemandirian Daya Surya:** Panel surya 10 Wp, TP4056 BMS, sel Li-ion 18650 1S4P.
5. **Casing 3D IoT Modular (IP54):** Enclosure PETG hasil cetak 3D oleh Darrel Alfath.
6. **Firmware FreeRTOS Multi-Task:** Task terpisah per fungsi (sensor, radio, aktuator, watchdog) dengan prioritas eksplisit — lihat `docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md`.
7. **Backend Go Berkinerja Tinggi:** MQTT Subscriber Worker Pool, Batch & Streaming ETL, Threshold Cache real-time (LISTEN/NOTIFY), REST API v1 enterprise-grade, WebSocket Hub, PostgreSQL/TimescaleDB dengan UUIDv7 konsisten.

---

## 3. Struktur Direktori Repositori Modular

```text
DESPRO2_SMART_SANITATION_MODULAR/
│
├── 00_MASTER_README.md
├── 01_MASTER_TASK_ALLOCATION.md
├── 02_BOM_PROCUREMENT.md
├── 03_MASTER_SEMESTER_WBS.md
├── 04_PHASE_GATE_AND_TRACEABILITY.md
├── 05_EVIDENCE_REGISTER.md
├── MANIFEST.json
├── README.md
│
├── docs/                                  # DOKUMENTASI ARSITEKTUR RESMI (SUMBER KEBENARAN)
│   ├── architecture/
│   │   ├── 00_ARCHITECTURE_DECISION_RECORD.md   # ⭐ Baca ini PERTAMA — pemersatu seluruh keputusan
│   │   ├── 01_DATABASE_ARCHITECTURE_AND_ERD.md
│   │   ├── 02_BACKEND_ETL_AND_API_ARCHITECTURE.md
│   │   ├── 03_FRONTEND_DASHBOARD_ARCHITECTURE.md
│   │   ├── 04_GATEWAY_ROUTER_NETWORK_PIPELINE.md
│   │   ├── 05_LORA_MQTT_TELEMETRY_PIPELINE.md
│   │   └── 06_REST_API_OPENAPI_SPEC.md
│   └── hardware_references/
│       └── 01_SENSOR_AND_ACTUATOR_REFERENCES.md
│
├── assets/
│   ├── cad/
│   ├── schematics/
│   ├── docs/
│   ├── logs/
│   ├── media/
│   ├── procurement/
│   └── sheets_templates/
│
├── reports/
│   ├── group/
│   └── individual/
│
└── src/                                   # IMPLEMENTASI KODE (MENGACU KE docs/architecture/)
    ├── bin/
    ├── config/
    │   ├── sensor_thresholds.json
    │   ├── lora_config.json
    │   ├── network_cpe220.conf
    │   ├── mosquitto.conf              # BARU — konfigurasi Broker (lihat 05_LORA_MQTT §7)
    │   ├── mosquitto_acl               # BARU — ACL per node
    │   └── .env.example
    ├── data/
    ├── firmware/                        # Firmware Node WC (FreeRTOS, LoRa-only)
    ├── firmware_gateway/                # BARU — Firmware terpisah untuk ESP32 Gateway
    └── server/
        ├── main.go
        ├── mqtt/subscriber.go          # BARU — MQTT Subscriber Worker Pool
        ├── models/, database/, etl/, api/
        └── static/index.html
```

---

## 4. Cara Menjalankan Sistem (Quick Start Guide — Revisi Docs-First)

> **Penting:** Urutan startup berubah karena MQTT kini menjadi prasyarat wajib, bukan opsional.

```powershell
# 1. Pastikan Mosquitto Broker sudah terinstal & berjalan (WAJIB sebelum Server Go)
mosquitto -c src\config\mosquitto.conf -v

# 2. Pindah ke direktori root repositori
cd c:\Users\dapah\Documents\DESPRO\DESPRO2_SMART_SANITATION_MODULAR

# 3. Jalankan binary server Go (akan otomatis connect ke Mosquitto sebagai subscriber)
.\src\bin\esos-server.exe
```

Setelah Broker dan Server aktif, buka peramban web pada:
👉 **`http://localhost:8000/`** atau **`http://192.168.0.100:8000/`** (via CPE220)

**Pengecekan kesehatan cepat:** `GET http://192.168.0.100:8000/api/v1/health` harus mengembalikan `{"status": "UP"}` — jika `DEGRADED`, cek koneksi `mqtt_broker` terlebih dahulu (lihat `06_REST_API_OPENAPI_SPEC.md` §4.6).

---

## 5. Susunan Tim & Kepemilikan Domain (Alokasi 205 Jam Proposal)

| No | Nama Anggota | NPM | Peran Proposal | Domain Tanggung Jawab Utama | Alokasi Proposal |
|:---:|:---|:---:|:---|:---|:---:|
| 1 | **Daffa Hardhan** | 2306161763 | Manajer Proyek dan Pengembang | Arsitektur sistem, Go backend (MQTT Subscriber, ETL), Web Dashboard, integrasi CPE220 & Mosquitto | **40 Jam** |
| 2 | **Raka Arrayan Muttaqien** | 2306161800 | Analis Solusi dan Integrasi Layanan | Requirement analysis, verifikasi standar sanitasi, evaluasi vendor BoM | **35 Jam** |
| 3 | **Siti Amalia Nurfaidah** | 2306161851 | Pengembang Perangkat Lunak | Firmware ESP32 FreeRTOS multi-task, ADC/PWM, LoRa RA-02 (RadioLib), kendali servo | **45 Jam** |
| 4 | **Muhammad Ilman Zuhriy** | 2306266786 | Perancang Perangkat Keras | Daya solar 10Wp + baterai 18650, TP4056, wiring harness, kalibrasi gas analog | **45 Jam** |
| 5 | **Darrel Alfath** | 2306266810 | Desainer Mekanis dan Penguji Kualitas | Desain 3D CAD casing IoT, weatherproofing IP54, Master Test Plan, QA | **40 Jam** |
| | **TOTAL AKUMULASI** | | | | **205 Jam** |

---

## 6. Indeks Navigasi Cepat Dokumen Master

- **[docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md](docs/architecture/00_ARCHITECTURE_DECISION_RECORD.md)** — ⭐ **Baca pertama.** Pemersatu seluruh keputusan arsitektur (ADR-01 s.d. ADR-08).
- [docs/architecture/01_DATABASE_ARCHITECTURE_AND_ERD.md](docs/architecture/01_DATABASE_ARCHITECTURE_AND_ERD.md) — ERD, strategi UUIDv7, threshold cache invalidation.
- [docs/architecture/02_BACKEND_ETL_AND_API_ARCHITECTURE.md](docs/architecture/02_BACKEND_ETL_AND_API_ARCHITECTURE.md) — MQTT Subscriber Worker Pool, ETL, threshold cache.
- [docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md](docs/architecture/03_FRONTEND_DASHBOARD_ARCHITECTURE.md) — Dashboard UI HTML5/Tailwind/Chart.js.
- [docs/architecture/04_GATEWAY_ROUTER_NETWORK_PIPELINE.md](docs/architecture/04_GATEWAY_ROUTER_NETWORK_PIPELINE.md) — Topologi jaringan, resiliensi Gateway.
- [docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md](docs/architecture/05_LORA_MQTT_TELEMETRY_PIPELINE.md) — Taksonomi topik MQTT, QoS, LWT, konfigurasi Broker.
- [docs/architecture/06_REST_API_OPENAPI_SPEC.md](docs/architecture/06_REST_API_OPENAPI_SPEC.md) — Kontrak REST API v1 enterprise-grade.
- [docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md](docs/hardware_references/01_SENSOR_AND_ACTUATOR_REFERENCES.md) — Task FreeRTOS, library, datasheet.
- [01_MASTER_TASK_ALLOCATION.md](01_MASTER_TASK_ALLOCATION.md) — Matriks RACI semester penuh.
- [02_BOM_PROCUREMENT.md](02_BOM_PROCUREMENT.md) — Kontrol pengadaan 15 komponen BoM.
- [03_MASTER_SEMESTER_WBS.md](03_MASTER_SEMESTER_WBS.md) — Master WBS 14 Pekan.
- [04_PHASE_GATE_AND_TRACEABILITY.md](04_PHASE_GATE_AND_TRACEABILITY.md) — Governance Gerbang 1-4 & RTM.
- [05_EVIDENCE_REGISTER.md](05_EVIDENCE_REGISTER.md) — Register bukti teknis.
- [MANIFEST.json](MANIFEST.json) — Metadata katalog repositori.
