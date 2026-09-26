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
| Pekan ke- | 5 (Lima) |
| Periode | 21 September 2026 – 26 September 2026 |
| Tanggal | 26 September 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

---

### B. Target Mingguan
Target yang direncanakan pada awal pekan berdasarkan WBS dan pembagian tugas kelompok.

| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pengujian Integrasi Modular Seluruh Sensor (Ultrasonik JSN-SR04T, Gas Amonia MQ-137, Gas H2S MQ-136, Servo MG996R) | Hasil uji bacaan sensor simultan & kestabilan nilai ADC | ☐ Selesai &nbsp; ☑ Belum |
| 2 | Investigasi & *Troubleshooting* Kendala Termal (*Heater Coil Overheating*) Sensor Gas | Karakterisasi lonjakan arus/tegangan & mitigasi thermal/power drop | ☑ Selesai &nbsp; ☐ Belum |
| 3 | *Debugging* & Perbaikan *Bug* Kode Firmware (Task FreeRTOS & Filter ADC Jitter) | Firmware C++ bebas *task starvation* & penerapan *moving average filter* 10 sampel | ☐ Selesai &nbsp; ☑ Belum |
| 4 | Analisis Penyelarasan Integrasi Radio LoRa & Manajemen Resiko Daya (*Brownout*) | Keputusan rekayasa sadar (*conscious engineering decision*) penundaan LoRa & desain *split power rail* | ☐ Selesai &nbsp; ☑ Belum |

---

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Senin, 21 Sep 2026 | Menyiapkan rencana uji integrasi serempak seluruh modul sensor (ultrasonik, MQ-137, MQ-136, servo) dan menyelaraskan pinout ESP32 bersama tim. | 2 | Catatan Skenario Uji |
| Selasa, 22 Sep 2026 | Melakukan pertemuan luring bersama Amel (Siti Amalia) di lab untuk merangkai seluruh sensor ke ESP32 dan memulai pengujian awal. | 3 | Foto Sesi Uji Lab |
| Rabu, 23 Sep 2026 | Melakukan investigasi intensif terhadap modul sensor gas MQ yang memanas berlebih (*sensor panas*); mengukur konsumsi arus pemanas dan mendeteksi penurunan tegangan drastis. | 3 | Data Pengukuran Arus & Tegangan |
| Kamis, 24 Sep 2026 | Menemukan akar permasalahan bahwa sumber catu daya baterai yang digunakan saat ini tidak sesuai spesifikasinya (tidak mampu memasok arus beban pemanas $\approx 350\text{ mA}$); mulai mencari alternatif solusi baterai bersama tim. | 2 | Catatan Investigasi Baterai |
| Jumat, 25 Sep 2026 | Melakukan *pair debugging* bersama Amel untuk menelusuri bug kode firmware C++ (penanganan ADC dan FreeRTOS); status kode masih dalam proses perbaikan sembari menunggu solusi sumber daya. | 2 | Draf Kode `main.cpp` |
| Sabtu, 26 Sep 2026 | Mengevaluasi status seluruh target pekan ke-5, mendokumentasikan temuan kendala baterai pada *risk register*, dan menyusun laporan kemajuan individu. | 2 | Laporan Individu Pekan 5 |

**Total Jam Kerja Minggu Ini : 14 Jam**

---

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan kelima ini, aktivitas yang **benar-benar telah berhasil diselesaikan secara tuntas** adalah:
- **Investigasi & Troubleshooting Kendala Termal Sensor Gas (Target 2 - Selesai 100%):**  
  Ketika menguji coba sensor gas amonia (MQ-137) dan hidrogen sulfida (MQ-136), ditemukan kendala fisik di mana sensor mengalami panas yang cukup tinggi (*heater coil overheating*). Melalui investigasi dan pengukuran kelistrikan mendalam menggunakan multimeter, saya berhasil mengidentifikasi akar penyebab utamanya: **sumber catu daya baterai yang ada saat ini tidak sesuai (*incompatible power source*)**. Baterai tidak mampu mempertahankan tegangan stabil saat dibebani arus kontinu pemanas sensor gas ($\approx 320\text{ mA} - 350\text{ mA}$), sehingga terjadi *voltage collapse* yang memicu panas berlebih pada regulator dan derau sinyal. Saat ini saya dan tim sedang mencari cara serta solusi alternatif untuk mengganti/menata ulang sumber baterai tersebut.

**Aktivitas yang Masih Berproses (Belum Selesai):**
- **Target 1 (Pengujian Integrasi Seluruh Sensor - Belum Selesai):** Pengujian integrasi penuh belum selesai karena pembacaan simultan tidak dapat berjalan stabil sebelum masalah sumber baterai teratasi.
- **Target 3 (Debugging Bug Kode Firmware - Belum Selesai):** Proses perbaikan bug pada task FreeRTOS dan filter ADC masih berlangsung (*work in progress*) bersama Amel.
- **Target 4 (Integrasi LoRa Fisik - Belum Selesai):** Integrasi modul komunikasi radio LoRa fisik belum dilakukan sama sekali pekan ini karena modul RF membutuhkan stabilitas daya tinggi, sehingga wajib menunggu penyelesaian masalah sumber baterai.

**2. Luaran yang Dihasilkan**
- Dokumen hasil investigasi kelistrikan yang membuktikan ketidaksesuaian sumber baterai terhadap beban pemanas sensor gas MQ.
- Spesifikasi teknis kebutuhan daya baterai baru (tegangan nominal, kapasitas mAh, dan *discharge current rating* yang dibutuhkan).
- Draf revisi kode firmware C++ (masih tahap pengembangan bersama Amel).

---

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Integrasi seluruh sensor & catu daya baterai | Ditemukan masalah *source* baterai tidak sesuai; sedang mencari solusi | 50% |
| Software (FW) | Pembenahan bug *timing* & filter ADC | Masih proses *debugging* bersama Amel | 45% |
| Software (Web) | Kesiapan backend menerima data riil | Sistem Go Backend & SCADA Dashboard siap (tuntas di W4) | 100% |
| Mekanik | Uji penempatan sensor dan baterai | Evaluasi dimensi baterai pengganti & ventilasi panas | 40% |
| Pengujian | Uji simultan fungsi sensor & transmisi | Baru tahap pengujian investigasi catu daya; uji LoRa belum | 35% |
| Dokumentasi | Pencatatan kendala baterai & logbook W5 | Logbook W5 selesai & *Risk Register* diperbarui | 100% |

**Ringkasan Kemajuan**
Pekan ini menjadi fase krusial dalam menemukan anomali mendasar pada sistem kelistrikan eSOS. Dari empat target yang direncanakan, target yang telah tuntas diselesaikan adalah investigasi kendala termal sensor gas, di mana akar masalahnya berhasil dilacak ke sumber catu daya baterai yang tidak sesuai spesifikasi. Tiga target lainnya (integrasi seluruh sensor, perbaikan bug firmware, dan integrasi LoRa) masih berstatus **Belum Selesai** karena sangat bergantung pada ketersediaan daya yang stabil. Langkah ini menghindarkan tim dari risiko kerusakan mikrokontroler (*brownout crash*) jika memaksakan integrasi tanpa memperbaiki sumber daya terlebih dahulu.

---

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | **Sumber Baterai Tidak Sesuai Spesifikasi Beban:** Baterai yang digunakan saat ini tidak mampu memasok arus kontinu untuk elemen pemanas (*heater coil*) sensor gas MQ-137 dan MQ-136 secara stabil. | Terjadi penurunan tegangan drastis (*voltage drop*), panas berlebih pada jalur catu daya, dan mikrokontroler tidak dapat beroperasi secara andal. | **Tinggi** |
| 2 | **Pengujian Integrasi Sensor & Transmisi LoRa Tertahan:** Modul LoRa fisik belum diintegrasikan dan pengujian pembacaan serempak seluruh sensor belum tuntas. | Linimasa integrasi nirkabel bergeser ke pekan berikutnya menunggu kepastian solusi catu daya baterai. | **Sedang** |
| 3 | **Bug Kode Firmware Masih dalam Proses Perbaikan:** Masih ditemukan *bug* pada sinkronisasi antrean FreeRTOS dan fluktuasi pembacaan ADC analog gas. | Kode firmware belum dapat dibekukan (*freeze*) untuk pengujian lapangan. | **Sedang** |

**Analisis Penyebab**
Elemen pemanas internal pada dua sensor MQ membutuhkan daya gabungan hingga $\approx 1.7\text{ Watt}$ secara terus-menerus. Sumber baterai yang saat ini tersedia memiliki kapasitas arus keluar (*internal resistance / discharge rate*) yang kurang memadai, sehingga ketika pemanas aktif, tegangan baterai langsung anjlok (*sagging*). Kondisi ini membuktikan bahwa kendala "sensor panas" dan ketidakstabilan sistem berpangkal pada ketidakcocokan sumber catu daya baterai.

---

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Ketidaksesuaian Sumber Baterai | Mencari alternatif sumber baterai yang sesuai (misalnya mengganti tipe sel baterai dengan rating *discharge* lebih tinggi, menggunakan konfigurasi paralel sel ganda, atau memisahkan modul regulator/step-up khusus untuk pemanas). | Daffa H. & Ilman | Pertengahan Pekan 6 |
| Bug Kode Firmware Belum Tuntas | Melanjutkan proses *pair debugging* bersama Amel untuk menyelesaikan algoritma *moving average filter* pada ADC dan penataan prioritas task FreeRTOS. | Siti Amalia & Daffa H. | Pekan 6 |
| Penundaan Integrasi LoRa Fisik | Menjadwalkan integrasi modul radio LoRa SX1278 segera setelah solusi sumber baterai terpasang dan terverifikasi mampu menahan beban arus transmisi. | Daffa H. & Siti Amalia | Akhir Pekan 6 |

---

### H. Dokumentasi Kemajuan
Berikut adalah bukti pendukung pelaksanaan aktivitas pekan kelima:

> **[BUKTI 1: INVESTIGASI KENDALA TERMAL & SUMBER DAYA BATERAI]**  
> *Foto sesi investigasi dan pengujian kelistrikan sensor gas bersama alat ukur multimeter/beban di laboratorium yang menunjukkan kendala pada sumber baterai.*  
> `![Investigasi Sumber Baterai](../media/w5_pengujian_sensor_panas.jpg)`

> **[BUKTI 2: PROSES DEBUGGING FIRMWARE BERSAMA AMEL]**  
> *Tangkapan layar lingkungan pengembangan PlatformIO / VS Code yang memperlihatkan proses perbaikan bug kode firmware yang masih berlangsung.*  
> `![Proses Debugging Bersama Amel](../media/w5_debugging_firmware_amel.png)`

> **[BUKTI 3: LOGBOOK HARIAN & EVALUASI TARGET W5]**  
> *Tangkapan layar pencatatan lembar kerja harian dan evaluasi deviasi target pada Google Sheets Pekan 5.*  
> `![Logbook Evaluasi Pekan 5](../media/w5_logbook_sheets_pekan5.png)`

---

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Kolaborasi Bersama Amel (Siti Amalia):** Melakukan sesi pengujian langsung di laboratorium dan bersama-sama menelusuri bug pada kode firmware C++.
- **Diskusi Sumber Daya Bersama Ilman:** Membahas hasil pengukuran tegangan baterai yang drop dan merumuskan kriteria penggantian sumber baterai yang cocok untuk pemanas sensor.
- **Transparansi Manajemen Proyek:** Mengomunikasikan status ketercapaian target kepada seluruh anggota tim bahwa integrasi LoRa ditunda demi keselamatan perangkat keras.

**Kontribusi Pribadi**
Memimpin investigasi teknis yang berhasil menemukan akar masalah ketidaksesuaian sumber baterai, mencegah kerusakan komponen akibat pemaksaan integrasi nirkabel, serta mendampingi proses *debugging* kode firmware.

---

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Investigasi kelistrikan berhasil mengungkap akar masalah sebenarnya (sumber baterai tidak sesuai) secara cepat dan akurat.
- Kerja sama tim dengan Amel berjalan sangat kompak dan komunikatif selama proses pelacakan kendala di laboratorium.

**Yang Perlu Diperbaiki**
- Perlu melakukan uji beban (*load test*) pada sumber baterai secara mandiri sebelum menghubungkannya ke rangkaian sensor kompleks.
- Pengelolaan waktu pengujian perlu dioptimalkan agar pencarian solusi alternatif baterai dapat dieksekusi lebih cepat.

**Pelajaran yang Didapat Minggu Ini**
- Kejujuran rekayasa (*engineering integrity*) sangat penting: mengakui target yang belum selesai akibat kendala teknis nyata jauh lebih berharga daripada memaksakan sistem berjalan tidak stabil.
- Kestabilan sumber daya adalah fondasi mutlak dari seluruh sistem IoT sebelum modul komunikasi nirkabel dapat dioperasikan.

---

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Menentukan dan menguji sumber baterai alternatif yang sesuai dengan kebutuhan arus pemanas sensor MQ. | Tegangan 5V tetap stabil tanpa drop saat seluruh pemanas aktif. | 4 Jam |
| 2 | Menuntaskan perbaikan bug kode firmware FreeRTOS bersama Amel. | Kode firmware C++ selesai diperbaiki dan siap uji integrasi. | 4 Jam |
| 3 | Melanjutkan pengujian integrasi seluruh sensor dan integrasi modul radio LoRa fisik. | Seluruh sensor terbaca simultan dan data terkirim via LoRa. | 6 Jam |

---

### L. Persetujuan
**Mahasiswa**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 26 September 2026  

**Ketua Kelompok**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 26 September 2026  

**Catatan Pembimbing**  
- "Bagus. Mengidentifikasi masalah sumber baterai yang tidak sesuai adalah temuan penting. Fokuskan pekan depan untuk menuntaskan solusi catu daya ini sebelum beralih ke transmisi LoRa."
