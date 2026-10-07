# LAPORAN PEKANAN INDIVIDU — DAFFA HARDHAN (PEKAN 1)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Daffa Hardhan  
NPM: 2306161763  
Peran Proposal: Manajer Proyek dan Pengembang (Alokasi: 40 Jam)  
Kelompok: 4 (Empat) | Pekan: 1 (Satu) | Periode: 26 Agustus 2026 – 01 September 2026  
Dosen Pembimbing: Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  

---

### A. IDENTITAS
- **Judul Proyek:** Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana
- **Kelompok:** 4 (Empat)
- **Pekan Pelaksanaan:** Pekan 1 (Satu)
- **Periode Waktu:** 26 Agustus 2026 – 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.

---

### B. TARGET MINGGUAN (PEKAN 1)
- Meninjau proposal akhir DP1 terkait domain arsitektur sistem dan manajemen.
- Menyusun Master WBS 14 Pekan, jadwal semester, dan alokasi tugas tetap (RACI 205 jam).
- Menetapkan struktur repositori modular proyek dan panduan pelaporan mingguan.
- Menyediakan artefak bukti awal untuk Laporan Kemajuan Kelompok Pekan 1.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 1: 20.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 26 Agt 2026 | Kick-off realisasi proposal, penelaahan arsitektur sistem eSOS | Catatan evaluasi arsitektur sistem | EV-W1-PM-001 | 4.0 |
| Kamis, 27 Agt 2026 | Penyusunan Master WBS semester 14 pekan dan jalur kritis | Berkas Master WBS (`03_...`) | EV-W1-PM-001 | 4.0 |
| Jumat, 28 Agt 2026 | Penetapan matriks alokasi tugas tetap (RACI 205 jam kerja) | Matriks alokasi tugas (`01_...`) | EV-W1-PM-003 | 4.0 |
| Sabtu, 29 Agt 2026 | Sinkronisasi spesifikasi protokol data LoRa dan backend server | Spesifikasi payload data JSON | EV-W1-PM-001 | 4.0 |
| Senin, 31 Agt 2026 | Konsolidasi dokumen perencanaan dan evaluasi Gate 1 | Dokumen Laporan Pekan 1 | EV-W1-REP-001 | 4.0 |
| **TOTAL PEKAN 1** | | | | **20.0 Jam** |

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W1-PM-01** | Evaluasi proposal DP1 | Evaluasi menyeluruh Bab 1 s.d. 5 tuntas diselaraskan ke DP2 | `DONE` | EV-W1-PM-001 |
| **W1-PM-04** | Master WBS terintegrasi | Dokumen WBS 14 Pekan selesai dan terkunci | `DONE` | EV-W1-PM-001 |
| **W1-PM-08** | Alokasi peran anggota | Matriks RACI fixed 205 jam kerja disepakati seluruh tim | `DONE` | EV-W1-PM-003 |
| **W1-PM-10** | Konsolidasi Laporan W1 | Laporan kemajuan kelompok Pekan 1 terbit resmi | `DONE` | EV-W1-REP-001 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Master WBS 14 Pekan pada `03_MASTER_SEMESTER_WBS.md` dan RACI fixed baseline pada `01_MASTER_TASK_ALLOCATION.md`.
- **Rincian Teknis:** Berhasil menerjemahkan proposal konseptual menjadi paket kerja operasional per pekan dengan pembagian domain tegas: Darrel (Casing 3D IoT), Ilman (Hardware & Power tanpa custom PCB), Siti (Firmware & LoRa), Raka (Pengadaan & Standar), dan Daffa (PM & Server).
- **Penerimaan Teknis:** Seluruh dokumen perencanaan disahkan pada evaluasi Phase Gate 1 (PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Perlunya standardisasi format payload data agar sinkron antara firmware LoRa Siti dan backend server.
- **Tindakan Korektif:** Menyusun spesifikasi payload biner tetap (*fixed-length byte array*) sebelum diubah ke JSON.
- **Hasil:** Struktur paket data disepakati dan siap diimplementasikan pada Pekan 2.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W1-PM-001`, `EV-W1-PM-002`, `EV-W1-PM-003`, `EV-W1-REP-001`.
- **Kontribusi:** Memimpin perencanaan semester, mengunci alokasi jam kerja tim, dan memastikan fondasi tata kelola repositori kokoh.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 20h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 2)
- Setup repositori Git resmi, aktivasi task tracker, persiapan server lokal, dan koordinasi Phase Gate 2.

---

### L. PERSETUJUAN
- **Mahasiswa:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
