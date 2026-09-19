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
| Pekan ke- | 4 (Empat) |
| Periode | 13 September 2026 – 19 September 2026 |
| Tanggal | 19 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

### B. Target Mingguan
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pembangunan & Redesain Web Dashboard SCADA (EMS) Full-Width | Antarmuka web SCADA mandiri dengan WebSocket & Chart.js live | [x] Selesai &nbsp; [ ] Belum |
| 2 | Pembuatan *Base Code* Firmware & Pendampingan *Wiring* Hardware | Kerangka kode C++ I/O & rangkaian kelistrikan dasar terpasang | [x] Selesai &nbsp; [ ] Belum |
| 3 | Evaluasi Logbook PM & Dokumentasi Uji Fungsional | Pembaruan Google Sheets W4 & *Evidence Register* | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Selasa, 15 Sep 2026 | Menulis *base code* (C++) untuk inisialisasi sensor analog dan motor servo ESP32. | 4 | *Screenshot Base Code* |
| Rabu, 16 Sep 2026 | Bertemu luring untuk membantu Ilman melakukan perakitan kabel (*wiring*) perangkat keras. | 4 | Foto Rangkaian Fisik |
| Jumat, 18 Sep 2026 | Mengembangkan arsitektur *frontend* Web Dashboard SCADA (EMS) dengan standar UI institusional. | 3 | Kode Sumber `index.html` |
| Sabtu, 19 Sep 2026 | Mengintegrasikan WebSocket real-time broadcast pada server Go, menguji simulasi data, dan rekap Sheets W4. | 3 | *Screenshot Dashboard EMS* & Sheets |

**Total Jam Kerja Minggu Ini : 14 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan keempat ini, saya berhasil merealisasikan dua pilar capaian penting:
1. **Front-End & SCADA Dashboard (EMS):** Mengembangkan dan merombak antarmuka Web Dashboard Pemantauan Lingkungan (*Environmental Monitoring System*) secara penuh (*100% Full-Width Fluid Canvas*). Dashboard ini mengadopsi standar visual SCADA profesional dengan palet warna Makara UI, metrik KPI real-time (level air, tegangan baterai, gas amonia, H2S), visualisasi grafik dinamis Chart.js, panel kendali katup pembilasan (idempotency UUIDv7), serta tabel audit paket telemetri live.
2. **Kolaborasi Cross-Functional (Hardware & Firmware):** Karena modul radio LoRa fisik sedang disempurnakan di tempat rekan tim (Siti), saya mengambil inisiatif menulis *base code* kerangka I/O aktuator/sensor serta membantu perakitan fisik (*wiring harness*) bersama Ilman.
3. **Manajemen Proyek:** Melakukan evaluasi logbook dan penelusuran deviasi target pada *Daily Logbook Tracker* Google Sheets.

**2. Luaran yang Dihasilkan**
- Antarmuka *Web Dashboard SCADA (EMS)* live di port `:8000` dengan dukungan WebSocket hub dan REST API.
- Generator simulasi data telemetri (`dummy_gateway.go`) untuk validasi *pipeline* tanpa ketergantungan perangkat keras.
- Kerangka kode sumber (*Base Code*) C++ ESP32.
- Pembaruan *Daily Logbook Tracker* W4.

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Bantuan *wiring* kelistrikan fisik modul | Komponen dasar tersambung jumper | 100% |
| Software (FW) | Pembuatan *Base Code* Aktuator & Sensor | Kode dasar selesai dan diserahkan ke tim | 100% |
| Software (Web) | Dashboard EMS SCADA Full-Width & WebSocket | Web UI terintegrasi dengan Go ETL live | 100% |
| Mekanik | - | - | 0% |
| Pengujian | Simulasi ingesti data telemetri real-time | Grafik bergerak dinamis tiap 2 detik | 100% |
| Dokumentasi | *Update* rekapitulasi pekan 4 | Logbook W4 terkonsolidasi | 100% |

**Ringkasan Kemajuan**
Meski pengujian transmisi lapangan nirkabel LoRa berjarak jauh ditunda menunggu perakitan akhir oleh rekan tim, penyelesaian subsistem Web Dashboard SCADA (EMS) secara mandiri membuktikan bahwa seluruh rantai pemrosesan data (Database $\rightarrow$ Server Go $\rightarrow$ WebSocket $\rightarrow$ Browser) telah berfungsi dengan sempurna.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Modul LoRa fisik masih berada di lokasi rekan tim untuk kalibrasi frekuensi radio, sehingga pengujian sinyal outdoor CPE220 ditunda. | Pengujian jaringan fisik tertunda satu pekan. | Rendah |

**Analisis Penyebab**
Komponen fisik belum disatukan di satu lokasi pengujian bersama. Namun deviasi ini telah dimitigasi dengan pengujian berbasis simulator telemetri (`dummy_gateway.go`).

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Penundaan Pengujian Lapangan | Menggantikan data transmisi riil dengan simulator injeksi data MQTT untuk memastikan Web Dashboard berfungsi 100%. Pengujian fisik dijadwalkan pada sesi integrasi Pekan 5. | Daffa H. | Pekan 5 |

### H. Dokumentasi Kemajuan
> **[BUKTI 1: SCREENSHOT DASHBOARD EMS SCADA REAL-TIME]**
> *Cara mengambil bukti: Buka browser ke `http://localhost:8000`, lalu screenshot layar penuh Dashboard EMS yang memperlihatkan kartu metrik, grafik bergerak, dan tabel log paket.*
> `![Bukti Web Dashboard EMS](../media/w4_daffa_ems_dashboard.png)`

> **[BUKTI 2: FOTO WIRING FISIK ATAU BASE CODE FIRMWARE]**
> *Cara mengambil bukti: Ambil foto rangkaian kabel/breadboard yang Anda bantu rakit bersama Ilman, atau screenshot file C++ base code.*
> `![Bukti Perakitan Hardware](../media/w4_daffa_wiring_hardware.jpg)`

> **[BUKTI 3: SCREENSHOT TRACKER SHEETS W4]**
> *Cara mengambil bukti: Screenshot Tab 'Daily_Logbook_Tracking' di Google Sheets untuk pekan keempat.*
> `![Bukti Live Tracking Sheets](../media/w4_daffa_sheets_tracking.png)`

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Sinergi Lintas Bidang:** Menyelaraskan *schema payload* JSON antara simulator backend dengan kode firmware C++ Siti.
- **Pendampingan Hardware:** Terjun langsung membantu Ilman dalam perakitan kabel kelistrikan untuk mempercepat persiapan modul fisik.

**Kontribusi Pribadi**
Menuntaskan pilar *frontend* dan visualisasi data SCADA (EMS) secara mandiri sehingga tim kini memiliki antarmuka demonstrasi yang konkret dan siap dipresentasikan kapan pun.

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Pembangunan Web Dashboard SCADA (EMS) berjalan sukses dengan performa latensi rendah berkat arsitektur WebSocket Go.
- Fleksibilitas peran membantu rekan tim berjalan efektif menjaga moral dan linimasa proyek.

**Yang Perlu Diperbaiki**
- Perlu segera menjadwalkan integrasi luring penuh di laboratorium agar data riil dari sensor fisik dapat langsung menggantikan simulator.

**Pelajaran yang Didapat Minggu Ini**
- Ketiadaan perangkat keras fisik di tangan tidak boleh menghentikan kemajuan perangkat lunak. Pembuatan *dummy simulator* terbukti menjadi strategi rekayasa yang sangat ampuh (*best practice*).

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Uji integrasi End-to-End modul fisik (LoRa Gateway $\rightarrow$ Mosquitto $\rightarrow$ Dashboard). | Data riil sensor tertampil di Web EMS. | 6 Jam |
| 2 | Persiapan demonstrasi purwarupa untuk Evaluasi Tengah Semester (Phase Gate 3). | Materi slide & video demo sistem. | 4 Jam |

### L. Persetujuan
**Mahasiswa**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 19 September 2026

**Ketua Kelompok**
Nama: Daffa Hardhan
Tanda tangan: *(Digital)*
Tanggal: 19 September 2026

**Catatan Pembimbing**
- "Oke. Jangan lupa dokumentasikan setiap tahap. Uji fungsional sangat penting, maka harus dicatat secara mendetail dalam *logbook* bagaimana metrik performanya (*performance*)."
