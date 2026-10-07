# LAPORAN PEKANAN INDIVIDU — DARREL ALFATH (PEKAN 1)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Darrel Alfath  
NPM: 2306266810  
Peran Proposal: Desainer Mekanis dan Penguji Kualitas (Alokasi: 40 Jam)  
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
- Meninjau batasan mekanik fasilitas sanitasi darurat dan standar ketahanan cuaca luar ruang IP54.
- Merancang konsep model 3D CAD casing / enclosure untuk perangkat IoT Smart-Sanitation eSOS.
- Merancang alur gasket sealing silikon RTV dan mekanisme linkage motor servo MG996R.
- Menyusun draf Master Test Plan penjaminan mutu mencakup skenario TC-01 sampai dengan TC-06.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 1: 20.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 26 Agt 2026 | Penelaahan batasan mekanik & ergonomi fasilitas sanitasi darurat | Dokumen batasan desain fisik | EV-W2-MECH-002 | 4.0 |
| Kamis, 27 Agt 2026 | Pemodelan konsep 3D CAD casing IoT dan housing sensor ultrasonik | File CAD part model v0.1 | EV-W2-MECH-002 | 4.0 |
| Jumat, 28 Agt 2026 | Perancangan mekanisme linkage transmisi putar motor servo MG996R | Desain linkage mekanik servo | EV-W2-MECH-002 | 4.0 |
| Sabtu, 29 Agt 2026 | Desain alur gasket sealing silikon RTV standar weatherproofing IP54 | Skema alur sealing enclosure | EV-W2-MECH-002 | 4.0 |
| Senin, 31 Agt 2026 | Penyusunan draf Master Test Plan (TC-01 s.d. TC-06) & evaluasi Gate 1 | Dokumen Test Plan v1.0 | EV-W2-QA-001 | 4.0 |
| **TOTAL PEKAN 1** | | | | **20.0 Jam** |

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W1-MQ-01** | Batasan desain fisik | Dokumen analisis ergonomi dan ketahanan impak casing tuntas | `DONE` | EV-W2-MECH-002 |
| **W1-MQ-02** | Konsep casing 3D IoT | Model 3D modular memuat ESP32, baterai, aktuator terancang | `DONE` | EV-W2-MECH-002 |
| **W1-MQ-05** | Standar IP54 | Desain lidah-alur (*tongue-and-groove*) untuk gasket silikon siap | `DONE` | EV-W2-MECH-002 |
| **W1-MQ-07** | Skenario pengujian QA | Master Test Plan 6 skenario (TC-01 s.d. TC-06) terdefinisi | `DONE` | EV-W2-QA-001 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Model 3D CAD konseptual casing IoT eSOS dan Master Test Plan awal (`04_PHASE_GATE_AND_TRACEABILITY.md`).
- **Rincian Teknis:** Berhasil merancang arsitektur casing modular yang menampung seluruh modul elektrikal standar tanpa memerlukan custom PCB kelompok. Casing memiliki kompartemen terisolasi antara baterai surya dan modul kontrol mikro.
- **Penerimaan Teknis:** Rancangan konsep mekanik disetujui pada evaluasi Phase Gate 1 (PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Penentuan posisi ventilasi sensor gas MQ-137/136 agar tetap terpapar udara namun terlindung dari semprotan air langsung.
- **Tindakan Korektif:** Menerapkan desain kisi ventilasi bertingkat (*louvered vents*) dengan sudut kemiringan $45^\circ$ ke bawah.
- **Hasil:** Sirkulasi gas lancar tanpa risiko masuknya percikan air dari atas.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-MECH-002`, `EV-W2-QA-001`.
- **Kontribusi:** Menghadirkan desain proteksi fisik yang tangguh dan menyusun kerangka penjaminan mutu terpadu bagi seluruh tim.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 20h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 2)
- Finalisasi gambar teknik 2D dimensi, ekspor CAD STEP/STL, persiapan cetak 3D PETG, dan penyiapan struktur bukti QA.

---

### L. PERSETUJUAN
- **Mahasiswa:** Darrel Alfath *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 01 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
