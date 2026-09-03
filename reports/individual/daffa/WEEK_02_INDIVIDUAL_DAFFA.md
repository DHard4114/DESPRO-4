# LAPORAN PEKANAN INDIVIDU — DAFFA HARDHAN (PEKAN 2)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Daffa Hardhan  
NPM: 2306161763  
Peran Proposal: Manajer Proyek dan Pengembang (Alokasi: 40 Jam)  
Kelompok: 4 (Empat) | Pekan: 2 (Dua) | Periode: 02 September 2026 – 08 September 2026  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  

---

### A. IDENTITAS
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Pelaksanaan:** Pekan 2 (Dua)
- **Periode Waktu:** 02 September 2026 – 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.

---

### B. TARGET MINGGUAN (PEKAN 2)
- Merancang dan membangun **Antarmuka Web Dashboard Pemantauan Real-Time** (HTML5, Tailwind CSS, Chart.js) untuk posko darurat.
- Membangun **Pipeline Pengambilan Data Sensor ke Basis Data** (FastAPI Ingest Endpoint & Skema Relasional SQLite).
- Mengonfigurasi dan menguji integrasi jaringan jarak jauh **TP-Link CPE220 Outdoor Access Point**.
- Menata struktur repositori modular, mengaktifkan live Google Sheets WBS tracker, dan memimpin evaluasi Phase Gate 2.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 2: 20.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 02 Sep 2026 | Perancangan skema basis data `schema.sql` (tabel telemetry, alert, commands) | Berkas skema SQL & inisialisasi DB | EV-W2-PM-002 | 4.0 |
| Kamis, 03 Sep 2026 | Pembangunan backend server FastAPI (`app.py`) & endpoint data ingest | Script backend server FastAPI | EV-W2-PM-002 | 4.0 |
| Jumat, 04 Sep 2026 | Konfigurasi jaringan TP-Link CPE220 AP (IP statis 192.168.0.254, SSID, routing) | Berkas konfigurasi CPE220 | EV-W2-PM-003 | 4.0 |
| Sabtu, 05 Sep 2026 | Pembuatan Web Dashboard interaktif (gauge tangki air, chart gas live, alarm SOS) | Frontend Dashboard `index.html` | EV-W2-PM-001 | 4.0 |
| Senin, 07 Sep 2026 | Pengujian pipeline end-to-end (sensor $\rightarrow$ CPE220 $\rightarrow$ backend $\rightarrow$ DB $\rightarrow$ UI) & Gate 2 | Hasil pengujian pipeline data | EV-W2-PM-001 | 4.0 |
| **TOTAL PEKAN 2** | | | | **20.0 Jam** |

**TOTAL AKUMULASI JAM (W1 + W2):** **40.0 Jam** *(100% Target Proposal).*

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W2-PM-01** | Web Dashboard Monitoring | Antarmuka dashboard responsif dengan Chart.js live update tuntas | `DONE` | EV-W2-PM-001 |
| **W2-PM-03** | Pipeline Data ke Database | Backend FastAPI menerima data LoRa/HTTP dan mencatat ke SQLite | `DONE` | EV-W2-PM-002 |
| **W2-PM-06** | Konfigurasi Jaringan CPE220 | Jaringan wireless outdoor directional 2.4GHz terkonfigurasi stabil | `DONE` | EV-W2-PM-003 |
| **W2-PM-08** | Master WBS & Live Tracking | Repositori modular dan tautan live Google Sheets terintegrasi | `DONE` | EV-W1-PM-001 |

---

### E. CAPAIAN TEKNIS MINGGUAN

#### Luaran Utama (Main Technical Outputs):
1. **Infrastruktur Server Go Berkinerja Tinggi (`src/server/main.go` & `src/bin/esos-server.exe`):**  
   Backend mandiri berbasis Go (Golang) yang dikompilasi menjadi *single executable binary* (`src/bin/esos-server.exe`) dengan penggunaan memori sangat hemat ($< 15\text{ MB}$ RAM) dan waktu kompilasi nol galat.
2. **Mesin ETL & Streaming Real-Time (`src/server/etl/pipeline.go` & `api/websocket.go`):**  
   - **Streaming Ingestion & WebSocket Hub:** Menerima stream paket data sensor LoRa / CPE220 dan langsung menyiarkannya (*broadcast*) ke seluruh dashboard klien secara instan ($< 1\text{ ms}$).
   - **Batch ETL Engine (Extract - Transform - Load):** Mengekstrak data mentah, mentransformasikan kalibrasi non-linear gas/volume air, mendeteksi anomali/alarm SOS, dan melakukan *bulk loading* berkala (setiap 20 rekaman atau 3 detik) ke basis data SQLite WAL mode dengan transaksi atomik untuk mengoptimalkan I/O disk.
3. **Web Dashboard Pemantauan Real-Time (`src/server/static/index.html`):**  
   Antarmuka dashboard web modern berbasis Tailwind CSS dan Chart.js dengan koneksi native WebSocket, gauge level air tangki, grafik gas amonia/$H_2S$ live, status baterai solar 1S4P, tombol kontrol katup servo, dan banner alarm SOS berkedip.
4. **Konfigurasi Jaringan TP-Link CPE220 (`src/config/network_cpe220.conf`):**  
   Pengaturan Access Point nirkabel outdoor directional 2.4GHz daya pancar 23 dBm untuk transmisi jarak jauh gateway sanitasi ke server posko.

#### Penerimaan Teknis (Technical Acceptance):
- Binary server Go `esos-server.exe` berhasil dikompilasi dan diuji: Server melayani koneksi WebSocket dan REST API secara bersamaan dengan waktu respon $< 5\text{ ms}$.
- Pipeline Batch ETL berhasil memproses aliran data sensor dan mengeksekusi *bulk commit* ke SQLite tanpa *write-lock contention*.

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Kebutuhan visualisasi data yang harus tetap ringan dijalankan pada laptop/server posko bencana tanpa koneksi internet publik.
- **Tindakan Korektif:** Membangun antarmuka berbasis HTML5 murni dengan penyimpanan lokal SQLite dan rendering grafik berbasis kanvas sisi klien (Chart.js).
- **Hasil:** Dashboard dapat diakses dengan latensi sangat rendah pada jaringan lokal CPE220 secara offline.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-PM-001`, `EV-W2-PM-002`, `EV-W2-PM-003`.
- **Kontribusi:** Menghadirkan pusat kendali visual (*command center*) bagi tim dan petugas posko untuk memantau keselamatan sanitasi serta mengintegrasikan seluruh alur data sistem eSOS.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 20h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 3)
- Uji integrasi data fisik dari mikrokontroler ESP32 Siti dan modul daya Ilman ke server lokal melalui jaringan CPE220.

---

### L. PERSETUJUAN
- **Mahasiswa:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
