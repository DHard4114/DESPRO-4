# TEMPLATE LAPORAN PEKANAN INDIVIDU
**Capstone Project Desain Proyek 2**

### A. Identitas
| Item | Keterangan |
|:---|:---|
| Nama | Daffa Hardhan |
| NPM | 2306161763 |
| Program Studi | Teknik Komputer |
| Kelompok | 4 (Empat) |
| Judul Proyek | Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana |
| Pekan ke- | 3 (Tiga) |
| Periode | 09 September 2026 – 12 September 2026 |
| Tanggal | 12 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

### B. Target Mingguan
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pembuatan Arsitektur Database PostgreSQL (UUIDv7) | File Skema DDL SQL & Tabel Siap Pakai | [x] Selesai &nbsp; [ ] Belum |
| 2 | Setup MQTT Subscriber Worker Pool | Skrip Go (`subscriber.go`) Terkoneksi Mosquitto | [x] Selesai &nbsp; [ ] Belum |
| 3 | Migrasi & Validasi Koneksi Mosquitto MQTT Lokal | Test Pub/Sub berhasil | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Rabu, 09 Sep 2026 | Riset UUIDv7 dan perancangan skema PostgreSQL untuk Time-Series Data | 3 | `postgres_schema.sql` |
| Kamis, 10 Sep 2026 | Eksekusi `postgres_schema.sql` pada lokal (esos_db) & revisi encoding seed data | 2 | Screenshot Database |
| Jumat, 11 Sep 2026 | Pemrograman Go (MQTT Worker Pool & Pipeline ETL) non-blocking channels | 5 | `src/server/etl/` & `mqtt/` |
| Sabtu, 12 Sep 2026 | Evaluasi capaian W3, tracking logbook anggota di Google Sheets, penulisan laporan | 4 | Bukti Commit Git & Sheets |

**Total Jam Kerja Minggu Ini : 14 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Telah berhasil menyelesaikan fondasi basis data PostgreSQL lokal yang menggunakan standar enterprise UUIDv7 dan tabel Time-Series untuk menampung telemetri sensor. Selain itu, subsistem *worker pool* MQTT menggunakan bahasa Go telah selesai direalisasikan untuk menelan antrean ribuan pesan dari Mosquitto tanpa *bottleneck*.

**2. Luaran yang Dihasilkan**
- Skema PostgreSQL SQL (`postgres_schema.sql`)
- Modul Go MQTT Subscriber (`subscriber.go`)
- Modul Go ETL Pipeline (`pipeline.go`)
- Tabel `esos_db` yang sudah termigrasi di laptop lokal

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | - | - | 0% |
| Software | Setup Database & Worker Pool MQTT | Database siap, ETL Go berhasil ingest data dummy | 100% |
| Mekanik | - | - | 0% |
| Pengujian | Test performa Pub/Sub lokal | Worker berhasil menelan QoS 1 secara konkuren | 100% |
| Dokumentasi | Pengecekan 205 Jam Logbook Sheets | Logbook W3 terkonsolidasi | 100% |

**Ringkasan Kemajuan**
Percepatan penyelesaian fase perangkat lunak. Secara teknis, pengembangan fitur backend untuk menampung sensor dan kendali servo (Worker Pool & DB) sudah selesai didesain melebihi target waktu yang ditentukan. Tim kini bisa fokus total pada firmware dan hardware fisik.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Isu encoding CRLF/LF yang membuat spasi berlebih pada seed data SQL. | Script seed data SQL gagal dieksekusi di OS Windows tanpa perbaikan manual. | Rendah |

**Analisis Penyebab**
Penyimpanan file teks `.sql` dengan format encoding *newline* Windows kadang memicu kesalahan spasi berlebih (garbled text) pada eksekusi terminal PostgreSQL (`psql`).

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Isu encoding SQL | Melakukan perbaikan manual (*Search and Replace*) pada baris *Seed Data* di file `postgres_schema.sql` lalu melakukan *push* ulang ke GitHub. | Daffa H. | Selesai Pekan 3 |

### H. Dokumentasi Kemajuan
> **[PLACEHOLDER BUKTI 1: SCREENSHOT DATABASE MIGRATION]**
> *Cara mengambil bukti: Buka Terminal (psql) atau DBeaver, perlihatkan tabel-tabel di `esos_db`.*
> `![Bukti Migrasi DB](../media/w3_daffa_db_tables.png)`

> **[PLACEHOLDER BUKTI 2: SCREENSHOT MQTT WORKER]**
> *Cara mengambil bukti: Jalankan server, screenshot terminal Go saat log Worker Pool menyala.*
> `![Bukti Go MQTT Worker](../media/w3_daffa_mqtt_worker.png)`

> **[PLACEHOLDER BUKTI 3: SCREENSHOT TRACKER SHEETS]**
> *Cara mengambil bukti: Screenshot Tab 'Daily_Logbook_Tracking' di Google Sheets minggu ini.*
> `![Bukti Live Tracking Sheets](../media/w3_daffa_sheets_tracking.png)`

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Diskusi teknis:** Sinkronisasi tipe data biner C++ dan JSON dengan Firmware Lead (Siti) untuk sinkronisasi `time_sync`.
- **Koordinasi administratif:** Mengawal disiplin anggota tim lain (Ilman, Darrel, Raka, Siti) untuk tertib mengisi *Live Tracking Sheets*.

**Kontribusi Pribadi**
Menyiapkan basis infrastruktur sistem IoT untuk menerima ratusan pesan dari Gateway ESP32, menyelesaikan desain skema Database, dan merawat tertib administrasi proyek (PM).

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Proses koding Worker Pool MQTT di Go sangat cepat dan minim *bug* fatal.
- Arsitektur UUIDv7 sukses diimplementasikan tanpa hambatan ekstensi *pgcrypto*.

**Yang Perlu Diperbaiki**
- Perlu merapikan dokumentasi konfigurasi TP-Link CPE220 untuk persiapan pengujian fisik *blank spot* (TC-03).

**Pelajaran yang Didapat Minggu Ini**
- Penggunaan *Goroutine* memecahkan masalah latensi koneksi MQTT secara drastis dibanding model *synchronous*.

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Finalisasi Web Dashboard (WebSocket) & Intersep Timestamp | UI Tervalidasi & Sinkronisasi Waktu | 4 Jam |
| 2 | Pengujian Jaringan Outdoor TP-Link CPE220 | Ping test stabil di area luar | 4 Jam |

### L. Persetujuan
**Mahasiswa**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 12 September 2026

**Ketua Kelompok**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 12 September 2026

**Catatan Pembimbing**
- 
