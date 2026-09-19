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
| 1 | Pembuatan *Base Code* Firmware Sensor & Aktuator | *Draft source code* ESP32 C++ untuk pembacaan I/O | [x] Selesai &nbsp; [ ] Belum |
| 2 | Bantuan Perakitan Fisik (*Wiring*) Komponen | Rangkaian kelistrikan terhubung | [x] Selesai &nbsp; [ ] Belum |
| 3 | Evaluasi Logbook PM & Dokumentasi Uji Fungsional | Pembaruan Google Sheets W4 & *Evidence Register* | [x] Selesai &nbsp; [ ] Belum |

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Selasa, 15 Sep 2026 | Menulis *base code* (C++) untuk inisialisasi sensor dan motor servo (aktuator). | 4 | *Screenshot Base Code* |
| Rabu, 16 Sep 2026 | Bertemu luring untuk membantu Ilman melakukan perakitan kabel (*wiring*) perangkat keras. | 4 | Foto Rangkaian Fisik |
| Sabtu, 19 Sep 2026 | Melengkapi konsolidasi logbook W4 di Google Sheets dan menyusun laporan deviasi. | 2 | Bukti Tracker Sheets W4 |

**Total Jam Kerja Minggu Ini : 10 Jam**

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan keempat ini, terdapat **penyesuaian rencana kerja**. Karena modul radio LoRa fisik saat ini masih dipegang oleh rekan setim untuk penyempurnaan, target pengujian jaringan transmisi CPE220 dan *Ping Test* harus ditunda. Untuk tetap menjaga progres pengerjaan agar linimasa tidak tertinggal, saya turun tangan lintas divisi (*cross-functional*):
1. **Software/Firmware:** Saya mengambil inisiatif menulis *base code* (kerangka dasar kode C++) untuk pembacaan sensor dan kendali aktuator ESP32. Kode dasar ini kemudian diserahkan kepada *Firmware Engineer* (Siti) untuk dilakukan *debugging* dan penyempurnaan.
2. **Hardware:** Saya membantu proses perakitan fisik kelistrikan (*wiring*) kabel *jumper* dan modul elektronika bersama Ilman.

**2. Luaran yang Dihasilkan**
- Kerangka kode sumber (*Base Code*) C++ untuk operasional I/O ESP32.
- Sebagian rangkaian fisik (*wiring*) modul keras terhubung.
- Pembaruan *Daily Logbook Tracker* W4.

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Bantuan *wiring* kelistrikan fisik | Modul dasar tersambung kabel | 100% |
| Software | Pembuatan *Base Code* Aktuator & Sensor | Kode berhasil ditulis untuk disempurnakan tim | 100% |
| Mekanik | - | - | 0% |
| Pengujian | - | Ditunda (menunggu modul LoRa) | 0% |
| Dokumentasi | *Update* rekapitulasi pekan 4 | Logbook W4 terkonsolidasi | 100% |

**Ringkasan Kemajuan**
Meski terjadi deviasi dari target awal (pengujian tertunda karena keterbatasan unit LoRa), partisipasi aktif (*cross-functional*) dalam ranah *hardware* dan *firmware* berhasil memastikan progres keseluruhan tim tetap berjalan dengan efektif.

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | Pengujian transmisi luar ruangan (CPE220 & Ping Test) belum bisa dilakukan karena modul radio LoRa masih dalam tahap penyempurnaan di lokasi anggota lain. | Jadwal uji coba lapangan mundur. | Sedang |

**Analisis Penyebab**
Komponen kritis (LoRa) belum siap 100% secara fisik di satu titik kumpul, sehingga integrasi sistem secara utuh (*End-to-End*) belum memungkinkan pekan ini.

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Penundaan Pengujian Lapangan | Mengalihkan alokasi jam kerja PM minggu ini untuk membantu percepatan *base code* firmware dan perakitan kabel (*wiring*) agar modul LoRa cepat selesai. Pengujian dijadwalkan ulang ke Pekan 5. | Daffa H. | Pekan 5 |

### H. Dokumentasi Kemajuan
> **[PLACEHOLDER BUKTI 1: SCREENSHOT KODE FIRMWARE (BASE CODE)]**
> *Cara mengambil bukti: Screenshot kode C++ (PlatformIO / Arduino IDE) bagian inisialisasi sensor atau aktuator servo yang Anda buat.*
> `![Bukti Base Code](../media/w4_daffa_base_code.png)`

> **[PLACEHOLDER BUKTI 2: FOTO WIRING / PERAKITAN]**
> *Cara mengambil bukti: Masukkan foto fisik dari kabel, breadboard, atau komponen ESP32 yang Anda bantu rakit.*
> `![Bukti Foto Wiring](../media/w4_daffa_wiring_hardware.jpg)`

> **[PLACEHOLDER BUKTI 3: SCREENSHOT TRACKER SHEETS W4]**
> *Cara mengambil bukti: Screenshot Tab 'Daily_Logbook_Tracking' di Google Sheets untuk pekan keempat.*
> `![Bukti Live Tracking Sheets](../media/w4_daffa_sheets_tracking.png)`

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Kerja Sama Lintas Divisi:** Turun langsung membantu Ilman di bagian *Hardware* (perakitan) dan membantu Siti di bagian *Firmware* (penulisan kode dasar).

**Kontribusi Pribadi**
Fokus menyelamatkan linimasa tim dengan mengambil porsi kerja rekan yang sedang mengalami *bottleneck*, memastikan bahwa anggota tim tidak kewalahan dan target proyek secara makro tetap tercapai.

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Kolaborasi tim berjalan luar biasa; pembagian beban kerja secara dinamis terbukti efektif mengatasi hambatan fisik komponen.

**Yang Perlu Diperbaiki**
- Perlu menjadwalkan kumpul luring (*offline*) secara penuh di mana semua komponen fisik dibawa ke satu tempat (MRPQ) agar pengujian integrasi bisa dilakukan.

**Pelajaran yang Didapat Minggu Ini**
- Dalam proyek *hardware*, fleksibilitas peran (*role flexibility*) sangat krusial ketika komponen fisik tertahan di satu orang. 

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Melakukan uji coba integrasi *End-to-End* (LoRa ke Gateway ke MQTT Server). | Data LoRa masuk ke PostgreSQL. | 5 Jam |
| 2 | Optimasi *Frontend* Web Dashboard & Persiapan Presentasi UTS (Phase Gate 3). | Antarmuka *Dashboard* Final & Materi Presentasi. | 5 Jam |

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
