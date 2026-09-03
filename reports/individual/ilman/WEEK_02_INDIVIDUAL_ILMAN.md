# LAPORAN PEKANAN INDIVIDU — MUHAMMAD ILMAN ZUHRIY (PEKAN 2)
## Despro 2 Smart-Sanitation eSOS

Status Dokumen: TEMPLATE TERKENDALI (CONTROLLED TEMPLATE)  
Nama Mahasiswa: Muhammad Ilman Zuhriy  
NPM: 2306266786  
Peran Proposal: Perancang Perangkat Keras (Alokasi: 45 Jam)  
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
- Memverifikasi spesifikasi elektrikal komponen fisik yang tiba di laboratorium terhadap datasheet.
- Mengunci topologi modul charger TP4056 BMS dengan konfigurasi 4 sel baterai Li-ion 18650 (1S4P).
- Memfinalisasi skematik pengkabelan terpusat dan instruksi perakitan wiring harness perfboard.
- Menguji stabilitas tegangan output rail 3.3V dan 5V pada simulasi kondisi beban penuh.

---

### C. LOGBOOK AKTIVITAS HARIAN (PEKAN 2: 19.0 JAM)
| Hari / Tanggal | Uraian Aktivitas Operasional | Luaran yang Dihasilkan | ID Bukti | Jam Kerja |
|:---|:---|:---|:---:|:---:|
| Rabu, 02 Sep 2026 | Verifikasi fisik komponen tiba terhadap datasheet dan ukur tegangan | Formulir inspeksi fisik hardware | EV-W2-PROC-002 | 4.0 |
| Kamis, 03 Sep 2026 | Perancangan tata letak wiring harness modular & uji kontinuitas | Diagram perutean wiring | EV-W2-HW-001 | 4.5 |
| Jumat, 04 Sep 2026 | Perakitan harness kabel daya, terminal blok, dan modul TP4056 | Sub-assembly elektrikal fisik | EV-W2-HW-004 | 4.5 |
| Sabtu, 05 Sep 2026 | Uji stabilitas tegangan output rail (3.3V/5V) pada beban penuh | Hasil pengukuran multimeter/DMM | EV-W2-HW-001 | 3.0 |
| Senin, 07 Sep 2026 | Pengujian integrasi modul daya surya bersama Darrel & verifikasi Gate 2 | Hasil uji ketahanan daya 24 jam | EV-W2-HW-002 | 3.0 |
| **TOTAL PEKAN 2** | | | | **19.0 Jam** |

**TOTAL AKUMULASI JAM (W1 + W2):** **45.0 Jam** *(100% Target Proposal).*

---

### D. REALISASI PEKERJAAN
| Kode Tugas | Target Spesifikasi | Realisasi Teknis yang Diselesaikan | Status | ID Bukti |
|:---|:---|:---|:---:|:---:|
| **W2-HW-01** | Verifikasi fisik komponen | 100% komponen daya dan sensor tiba tervalidasi parameter DMM | `DONE` | EV-W2-PROC-002 |
| **W2-HW-04** | Validasi solar 10Wp & baterai | Pembangkitan panel surya dan baterai 18650 teruji siap operasi | `DONE` | EV-W2-HW-002 |
| **W2-HW-05** | Topologi baterai 1S4P | Susunan baterai 12000 mAh dengan dual BMS charger terkunci aman | `DONE` | EV-W2-HW-003 |
| **W2-HW-10** | Rencana perakitan wiring | Wiring harness perfboard modular terisolasi siap dipasang | `DONE` | EV-W2-HW-004 |

---

### E. CAPAIAN TEKNIS MINGGUAN
- **Luaran Utama:** Skematik kelistrikan final (`assets/schematics/schematic_v1.pdf`), sub-assembly rangkaian daya solar, dan diagram topologi baterai.
- **Rincian Teknis:** Berhasil memvalidasi modul pengisian daya baterai 1S4P Li-ion 18650 melalui modul charger TP4056 dengan arus pengisian maksimum $2.0\text{ A}$. Pengukuran multimeter membuktikan tegangan keluaran rel 3.3V dan 5V memiliki regulasi tegangan sangat baik ($\pm 0.5\%$) dengan tegangan riak di bawah $12\text{ mV}$ saat aktuator aktif.
- **Penerimaan Teknis:** Kriteria perangkat keras Phase Gate 2 dinyatakan lulus (G2.2: PASS).

---

### F. PERMASALAHAN, KENDALA & G. TINDAKAN KOREKTIF
- **Kendala:** Penggunaan kabel jumper biasa rentan mengalami kelonggaran kontak (*loose contact*) saat terkena getaran mekanik.
- **Tindakan Korektif:** Menggunakan kabel berisolasi AWG22 berkualitas tinggi dengan terminal blok screw terisolasi dan pembungkus heat-shrink.
- **Hasil:** Rangkaian wiring harness sangat kokoh, rapi, dan tahan getaran.

---

### H. DOKUMENTASI & I. KONTRIBUSI TIM
- **Bukti Terkait:** `EV-W2-HW-001`, `EV-W2-HW-002`, `EV-W2-HW-003`, `EV-W2-HW-004`.
- **Kontribusi:** Menghadirkan sistem suplai daya surya mandiri yang kokoh dan bebas dari ketergantungan fabrikasi custom PCB.

---

### J. EVALUASI DIRI (SKOR: 25/25 POIN)
- Kehadiran (5/5), Jam Kerja 19h (5/5), Kontribusi Teknis (5/5), Kerja Sama (5/5), Profesionalisme (5/5).

---

### K. RENCANA PEKAN BERIKUTNYA (PEKAN 3)
- Perakitan fisik sirkuit daya ke dalam casing 3D PETG buatan Darrel dan karakterisasi kurva ppm sensor gas analog.

---

### L. PERSETUJUAN
- **Mahasiswa:** Muhammad Ilman Zuhriy *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Ketua Kelompok:** Daffa Hardhan *(Tanda Tangan)* — Tanggal: 08 September 2026
- **Dosen Pembimbing:** Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.
