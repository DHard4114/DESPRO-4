# LAPORAN PEKANAN INDIVIDU — DARREL ALFATH (PEKAN 2)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Darrel Alfath  
NPM: 2306266810  
Peran Proposal: Desainer Mekanis dan Penguji Kualitas (Alokasi: 40 Jam)  
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
- Memfinalisasi gambar teknik manufaktur 2D lengkap dengan toleransi dimensi ruang komponen.
- Mengunci model 3D CAD casing IoT eSOS dan mengekspor berkas format STEP serta STL siap cetak.
- Melakukan verifikasi ketersediaan material filamen PETG dan silicone sealant RTV.
- Menyiapkan template pengujian penjaminan mutu (QA) dan struktur direktori bukti pengujian.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 2: 20.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 02 Sep 2026 | Finalisasi gambar teknik manufaktur 2D lengkap dengan toleransi | Lembar gambar teknik dimensi | EV-W2-MECH-001 | 4.0 |
| Kamis, 03 Sep 2026 | Ekspor berkas CAD 3D parametrik (STEP/STL) & konfigurasi slicer | Berkas 3D CAD & file STL siap print | EV-W2-MECH-002 | 4.0 |
| Jumat, 04 Sep 2026 | Verifikasi material PETG, setting printer 3D, dan slicing g-code | Berkas g-code cetak 3D casing | EV-W2-MECH-002 | 4.0 |
| Sabtu, 05 Sep 2026 | Simulasi fit-check modul ESP32/sensor di CAD & uji waterproofing | Log simulasi ruang dalam casing | EV-W2-MECH-001 | 4.0 |
| Senin, 07 Sep 2026 | Penyiapan form pengujian QA TC-01..06 dan evaluasi Gate 2 | Template lembar kerja QA | EV-W2-QA-001 | 4.0 |
| **TOTAL PEKAN 2** | | | | **20.0 Jam** |

**TOTAL AKUMULASI JAM (W1 + W2):** **40.0 Jam** *(100% Target Proposal).*

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W2-MECH-01** | Gambar teknik manufaktur | Gambar 2D dimensi dan toleransi fit terverifikasi | `DONE` | EV-W2-MECH-001 |
| **W2-MECH-02** | Dimensi casing terkunci | Ruang dalam casing memuat modul breakout, baterai, sensor | `DONE` | EV-W2-MECH-001 |
| **W2-MECH-06** | Berkas CAD 3D release | File `.step` dan `.stl` tersimpan di `assets/cad/` | `DONE` | EV-W2-MECH-002 |
| **W2-MECH-08** | Rencana cetak 3D PETG | Parameter slicing dan material filamen tervalidasi | `DONE` | EV-W2-MECH-002 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Berkas 3D CAD parametrik (`assets/cad/sanitation_modular_v1.step`), gambar teknik 2D (`assets/cad/technical_drawing_2d.pdf`), dan berkas STL siap cetak.
- **Rincian Teknis:** Berhasil memfinalisasi desain casing 3D enclosure IoT Smart-Sanitation eSOS dengan ketebalan dinding nominal $3.0\text{ mm}$ berbahan filamen PETG. Casing dilengkapi mounting standoff M3 terdedikasi untuk modul ESP32, housing sensor ultrasonik JSN-SR04T di bagian bawah, kompartemen baterai 18650, dan alur gasket silikon RTV $2.5\text{ mm}$ untuk proteksi IP54.
- **Penerimaan Teknis:** Kriteria mekanikal Phase Gate 2 disetujui (G2.5: PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Potensi deformasi sudut (*warping*) pada casing berbidang datar saat pencetakan 3D.
- **Tindakan Korektif:** Mengatur suhu bed printer pada 85°C dan menambahkan struktur brim selebar 8mm pada konfigurasi slicer.
- **Hasil:** Simulasi g-code dan parameter cetak tervalidasi siap difabrikasi di lab pada Pekan 3.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-MECH-001`, `EV-W2-MECH-002`, `EV-W2-QA-001`.
- **Kontribusi:** Menyiapkan casing 3D IoT kustom yang presisi dan kokoh serta menyusun sistem evaluasi mutu bagi seluruh pengujian sistem eSOS.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 20h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 3)
- Proses pencetakan 3D casing PETG di lab, aplikasi sealing silikon RTV, dan perakitan fisik modul hardware bersama Ilman.

---

### L. PERSETUJUAN
- **Mahasiswa:** Darrel Alfath *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
