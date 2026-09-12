# Laporan Individu Mingguan (Week 3)
**Desain Proyek 2 (Gasal 2026/2027)**

*   **Nama:** Daffa Hardhan
*   **NPM:** 2306161763
*   **Peran:** Manajer Proyek & Pengembang Backend (Go)
*   **Periode Evaluasi:** 09 September 2026 – 12 September 2026 (Pekan 3)
*   **Status Kehadiran Rapat:** Hadir

---

## 1. Rangkuman Target & Capaian (Sesuai WBS)

Berdasarkan *Master WBS* (`03_MASTER_SEMESTER_WBS.md`) Fase 3, target pekerjaan saya pada pekan ini difokuskan pada fondasi arsitektur Backend dan tata kelola proyek:

| ID Tugas WBS | Deskripsi Pekerjaan (Target) | Status Capaian | Persentase |
| :--- | :--- | :--- | :---: |
| `W3-PM-01` | Pengembangan skema Backend Database lokal (PostgreSQL) menggunakan arsitektur UUIDv7 & Trigger `LISTEN/NOTIFY`. | Selesai | 100% |
| `W3-PM-02` | Setup & Migrasi tabel utama (`telemetry_records`, `sanitation_nodes`, dll.) ke database lokal (`esos_db`). | Selesai | 100% |
| `W3-PM-03` | Pembuatan *skeleton* dan arsitektur *MQTT Subscriber Worker Pool* berbasis Goroutine (`subscriber.go` & `pipeline.go`). | Selesai | 100% |
| `W3-PM-04` | *Project Management*: Mengawasi kepatuhan tim terhadap *Google Sheets Live Tracker* dan melakukan konsolidasi Git. | Selesai | 100% |

---

## 2. Rincian Pelaksanaan Tugas & Bukti Teknis (*Evidence*)

### A. Implementasi Database PostgreSQL & UUIDv7
Telah diselesaikan perancangan DDL (Data Definition Language) dan penyusunan fungsi kustom `uuid_generate_v7()` agar performa B-Tree pada basis data *time-series* tetap tinggi (menghindari fragmentasi indeks pada PostgreSQL). Semua tabel telah dimigrasi ke database lokal.

> **[PLACEHOLDER BUKTI 1: SCREENSHOT DATABASE]**
> *Cara mengambil bukti: Buka pgAdmin4, DBeaver, atau Terminal. Tampilkan daftar tabel (`\dt`) pada database `esos_db` yang sudah berhasil dimigrasi.*
> 
> `![Bukti Migrasi Database PostgreSQL](../media/w3_daffa_db_tables.png)`

### B. Arsitektur MQTT Subscriber Worker Pool (Golang)
Sistem ETL (*Extract, Transform, Load*) untuk menelan antrean ribuan pesan MQTT (*QoS 1*) secara konkuren telah berhasil diimplementasikan. Arsitektur menggunakan model *Buffered Channel* `InStream` dan `OutStream` yang diproses oleh sekumpulan Goroutine sehingga bersifat *non-blocking*.

> **[PLACEHOLDER BUKTI 2: SCREENSHOT KODE / LOG EKSEKUSI WORKER]**
> *Cara mengambil bukti: Jalankan `go run main.go` atau perlihatkan *screenshot* VS Code pada file `pipeline.go` / `subscriber.go` yang menunjukkan Goroutine Workers berhasil diluncurkan.*
> 
> `![Bukti MQTT Worker Pool Go](../media/w3_daffa_go_worker.png)`

### C. Manajemen Proyek & Tracking Logbook
Telah dilakukan pemantauan terhadap seluruh pergerakan tugas pada *Google Sheets Live Master WBS Tracker* dan memastikan dokumen *Evidence Register* siap menerima *input* QA dari Darrel dan perakitan dari Ilman/Siti.

> **[PLACEHOLDER BUKTI 3: SCREENSHOT GOOGLE SHEETS]**
> *Cara mengambil bukti: Ambil *screenshot* tab `Daily_Logbook_Tracking` pada Google Sheets yang menunjukkan Anda telah memeriksa jam kerja pekan ke-3.*
> 
> `![Bukti Live Tracking Sheets](../media/w3_daffa_sheets_tracking.png)`

---

## 3. Kendala & Mitigasi Risiko

*   **Kendala (Pekan Ini):** Ditemukan anomali *encoding* pada *seed data* di file `.sql` akibat perbedaan pemrosesan *newline* antara OS (CRLF vs LF).
*   **Mitigasi:** Telah dilakukan perbaikan langsung via *Git Commit* sehingga file `postgres_schema.sql` saat ini sepenuhnya steril dan bisa dijalankan pada mesin *Windows* bawaan tanpa *error syntax*.

## 4. Target Rencana Pekan Depan (Week 4)

*   Melanjutkan *stress testing* koneksi antara *Mosquitto MQTT Broker* lokal dengan *Go Backend Worker*.
*   Membantu Siti Amalia (Firmware) dalam melakukan intersep sinkronisasi waktu (*True Timestamping*) dari *server* ke ESP32 Gateway via MQTT `time_sync`.
*   Integrasi akhir UI Web Dashboard (*WebSocket*) dengan API yang telah dibuat.

---
**Dibuat Oleh:**
*(Daffa Hardhan)*
12 September 2026
