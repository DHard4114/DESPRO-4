# LAPORAN PEKANAN INDIVIDU — DAFFA HARDHAN (PEKAN 2)
## Despro 2 Smart-Sanitation eSOS — Kelompok 4 DTE FTUI

Status Dokumen: LAPORAN RESMI TERKENDALI (CONTROLLED BASELINE)  
Nama Mahasiswa: Daffa Hardhan  
NPM: 2306161763  
Peran Proposal: Manajer Proyek dan Pengembang (Alokasi Proposal: 40 Jam Semester)  
Pekan Ke-: 2 (Dua) | Periode: 02 September 2026 – 08 September 2026  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  

---

### A. IDENTITAS & FOKUS PEKAN 2
Sesuai dengan CPMK Pekan 2 (*"Menyiapkan seluruh sumber daya yang diperlukan untuk mewujudkan sistem yang dirancang"*), tugas utama Daffa Hardhan pada pekan ini adalah:
1. Menyiapkan dan mengelola repositori Git resmi proyek di GitHub (`https://github.com/DHard4114/DESPRO-4.git`).
2. Menginisiasi pembuatan infrastruktur backend server Go (Golang) berkinerja tinggi (`src/server/main.go`), binary mandiri (`src/bin/esos-server.exe`), mesin ETL (Streaming & Batch), serta basis data lokal SQLite WAL Mode dengan kunci UUIDv7 (`src/data/esos_telemetry.db`).
3. Mengembangkan antarmuka Web Dashboard pemantauan real-time (`src/server/static/index.html`) dengan koneksi WebSocket.
4. Menyiapkan konfigurasi router nirkabel outdoor TP-Link CPE220 (`src/config/network_cpe220.conf`).
5. Mengintegrasikan template Master WBS 14 pekan dan logbook harian 205 jam di Google Sheets.

---

### B. LOGBOOK AKTIVITAS HARIAN (PEKAN 2)

| Hari / Tanggal | Jam Kerja | Durasi | Uraian Aktivitas Nyata | Luaran Teknis / Bukti | ID Bukti |
|:---|:---:|:---:|:---|:---|:---:|
| Selasa, 01 Sep 2026 | 09:00 – 17:00 | 8.0 Jam | Inisiasi backend server Go (`src/server/`), implementasi Streaming Ingestion, Batch ETL Engine, dan kompilasi single binary `src/bin/esos-server.exe`. | Binary `esos-server.exe` & Go Code | `EV-W2-PM-002` |
| Rabu, 02 Sep 2026 | 10:00 – 17:00 | 7.0 Jam | Pengembangan Web Dashboard pemantauan real-time (`index.html`), visualisasi chart telemetri WebSocket, konfigurasi router CPE220, dan setup Git GitHub. | `src/server/static/index.html` & Git Repo | `EV-W2-PM-001` |
| **TOTAL PEKAN 2** | | **15.0 Jam** | | | |

* **Realisasi Pekan 1:** 14.0 Jam
* **Realisasi Pekan 2:** 15.0 Jam
* **Total Akumulasi Terlaksana (W1 + W2):** **29.0 Jam** *(dari total komitmen 40 Jam proposal semester penuh).*

---

### C. REALISASI PEKERJAAN & LUARAN TEKNIS

| Kode Tugas | Target CPMK Pekan 2 | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W2-PM-01** | Setup Repositori Git Proyek | Inisialisasi Git, branch structure, .gitignore, dan push ke GitHub `DESPRO-4` | `DONE` | `EV-W1-PM-001` |
| **W2-PM-02** | Konfigurasi Server & Database | Inisiatif Go server, SQLite WAL Mode, DDL PostgreSQL, UUIDv7 Primary Key | `DONE` | `EV-W2-PM-002` |
| **W2-PM-03** | Web Dashboard Monitoring UI | Antarmuka dashboard responsif (Tailwind, Chart.js) dengan WebSocket Hub | `DONE` | `EV-W2-PM-001` |
| **W2-PM-04** | Konfigurasi Router Jaringan | Pengaturan IP statis dan nirkabel directional outdoor TP-Link CPE220 | `DONE` | `EV-W2-PM-003` |
| **W2-PM-05** | Master WBS & Task Tracker | Lembar Sebar Google Sheets 5 Tabs terhubung penuh ke repositori | `DONE` | `EV-W1-PM-001` |

---

### D. CAPAIAN UTAMA (KEY DELIVERABLES)

1. **Infrastruktur Server Go Mandiri (`src/bin/esos-server.exe`):**  
   Server native binary yang sangat ringan ($< 15\text{ MB}$ RAM) tanpa memerlukan instalasi interpreter tambahan, siap dijalankan langsung di posko darurat.
2. **Mesin ETL & WebSocket Hub Real-Time (`src/server/etl/pipeline.go`):**  
   Menerima stream paket sensor dan menyiarkannya ke dashboard secara instan ($< 1\text{ ms}$) serta melakukan batch insert atomik ke SQLite WAL mode.
3. **Repositori GitHub Terkendali:**  
   Seluruh arsitektur sistem, 5 master docs, dan CSV templates telah aktif di `https://github.com/DHard4114/DESPRO-4.git`.

---

### E. RENCANA KERJA PEKAN 3
1. Menyiapkan simulasi payload sensor terpadu ke endpoint ingestion backend.
2. Memantau fabrikasi 3D printing casing oleh Darrel dan perakitan wiring daya oleh Ilman.
3. Memperbarui persentase kemajuan WBS Pekan 3 di Google Sheets.

---

| Tanda Tangan Mahasiswa | Tanggal Pengesahan |
|:---:|:---:|
| *(Daffa Hardhan — 2306161763)* | 08 September 2026 |
