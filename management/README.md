# Manajemen Proyek Capstone: Smart-Sanitation eSOS 📋

Direktori ini berisi seluruh dokumen tata kelola, perencanaan, pengadaan, dan pelaporan berkala untuk proyek **Smart-Sanitation eSOS (Emergency Sanitation Operating System)** - Kelompok 4 Desain Proyek 2, Departemen Teknik Elektro FTUI (2026).

---

## 🗂️ Struktur Dokumen Manajemen

```
management/
├── 01_MASTER_TASK_ALLOCATION.md         # Matriks Alokasi Tugas & RACI Kelompok
├── 02_BOM_PROCUREMENT.md                # Bill of Materials (BOM) & Status Pengadaan Komponen
├── 03_MASTER_SEMESTER_WBS.md            # Work Breakdown Structure (WBS) & Jadwal Semester
├── 04_PHASE_GATE_AND_TRACEABILITY.md    # Phase Gate Deliverables & Matriks Ketertelusuran
├── 05_EVIDENCE_REGISTER.md              # Register Bukti Fisik, Foto Lab, & Hasil Pengujian
└── reports/                             # Arsip Laporan Berkala (Weekly Progress)
    ├── group/                           # Laporan Kelompok Mingguan (Week 01 - 04)
    ├── individual/                      # Laporan Individu per Anggota (Daffa, Darrel, Ilman, Raka, Siti)
    └── media/                           # Foto Dokumentasi Perakitan & Kalibrasi Lab
```

---

## 📑 Ringkasan Isi Dokumen

### 1. [01_MASTER_TASK_ALLOCATION.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/01_MASTER_TASK_ALLOCATION.md)
*   **Fokus:** Pembagian peran, tanggung jawab, dan matriks RACI (*Responsible, Accountable, Consulted, Informed*) 5 anggota tim.
*   **Peran Utama:**
    *   **Daffa:** Project Lead, Arsitektur Sistem, FreeRTOS Firmware Gateway, Integrasi LoRa.
    *   **Darrel:** Perancangan Mekanikal & Casing Bilik WC, Integrasi Solenoid / Servo Lock.
    *   **Ilman:** Power System (Solar PV 20 Wp, TP4056/BMS, Baterai Li-Ion 18650), Desain PCB.
    *   **Raka:** Backend Golang, PostgreSQL Database ACID, Mosquitto MQTT Broker, REST API.
    *   **Siti:** Firmware Node Sensor (Sensor Ultrasonik JSN-SR04T, MQ-137 Ammonia, MQ-136 H2S, Tombol SOS).

### 2. [02_BOM_PROCUREMENT.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/02_BOM_PROCUREMENT.md)
*   **Fokus:** Daftar belanja suku cadang teknis, vendor, spesifikasi kuantitas, harga, dan status verifikasi fisik di laboratorium.
*   **Komponen Kunci:** Semtech SX1278 (433MHz Ra-02), ESP32-WROOM-32 DevKit V1, RTC DS3231, Sensor JSN-SR04T, MQ-137/136, Servo Metal Gear MG996R, Solenoid 12V.

### 3. [03_MASTER_SEMESTER_WBS.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/03_MASTER_SEMESTER_WBS.md)
*   **Fokus:** Rincian paket kerja (Work Breakdown Structure) bertingkat dari Level 1 (Sistem) hingga Level 4 (Sub-tugas) dengan jadwal *milestone* semester genap 2026.

### 4. [04_PHASE_GATE_AND_TRACEABILITY.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/04_PHASE_GATE_AND_TRACEABILITY.md)
*   **Fokus:** Titik verifikasi fase (Gate 1 hingga Gate 4), kriteria kelulusan teknis, dan matriks ketertelusuran kebutuhan (*Requirements Traceability Matrix*).

### 5. [05_EVIDENCE_REGISTER.md](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/05_EVIDENCE_REGISTER.md)
*   **Fokus:** Log bukti pengujian hardware, output serial monitor, hasil uji RF LoRa, log database PostgreSQL, dan referensi berkas visual.

### 6. [reports/](file:///c:/Users/dapah/Documents/DESPRO/DESPRO2_SMART_SANITATION_MODULAR/management/reports)
*   Arsip mingguan pelaporan Capstone untuk dosen pembimbing dan koordinator mata kuliah, mencakup jam kerja, tantangan teknis, dan target pekan berikutnya.

---

> [!NOTE]
> Seluruh berkas dalam direktori ini ditujukan murni untuk keperluan **manajemen, administrasi, dan pelaporan akademik**. Seluruh spesifikasi teknis dan implementasi kode sumber berada di direktori `docs/` dan `src/`.
