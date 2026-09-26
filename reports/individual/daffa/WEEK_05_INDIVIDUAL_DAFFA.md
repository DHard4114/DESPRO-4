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
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pengujian Integrasi Modular Seluruh Sensor (Ultrasonik & Gas MQ) pada ESP32 | Seluruh sensor terhubung I/O dan menghasilkan pembacaan lokal | [x] Selesai &nbsp; [ ] Belum |
| 2 | Investigasi & *Troubleshooting* Kendala Termal (*Heater Coil Overheating*) Sensor Gas | Karakterisasi beban arus pemanas MQ dan rencana isolasi daya | [x] Selesai &nbsp; [ ] Belum |
| 3 | *Debugging* & Perbaikan *Bug* Kode Firmware (Task FreeRTOS & ADC Jitter) | Kode C++ firmware stabil tanpa *watchdog reset* / *memory leak* | [x] Selesai &nbsp; [ ] Belum |
| 4 | Penyelarasan Integrasi Radio LoRa & Manajemen Resiko Daya (*Brownout*) | Keputusan teknis penundaan LoRa demi mitigasi *power sag* | [x] Selesai &nbsp; [ ] Belum |

---

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Senin, 21 Sep 2026 | Melakukan evaluasi progres W4 bersama tim dan menyusun skenario uji integrasi modular seluruh sensor (JSN-SR04T, MQ-137, MQ-136, dan servo) sebelum tahap transmisi RF. | 2 | Catatan Skenario Uji |
| Selasa, 22 Sep 2026 | Bertemu luring bersama Amel (Siti Amalia) di lab untuk merangkai dan menguji coba integrasi pembacaan seluruh sensor secara serempak pada mikrokontroler ESP32. | 3 | Foto Rangkaian Uji Luring |
| Rabu, 23 Sep 2026 | Mengidentifikasi kendala termal (*sensor panas*) pada modul MQ-137 dan MQ-136 akibat kumparan pemanas (*internal heater coil*), serta mengukur lonjakan arus yang membebani rel daya 5V. | 3 | Data Pengukuran Arus & Suhu |
| Kamis, 24 Sep 2026 | Melakukan *pair debugging* bersama Amel pada kode firmware C++: membenahi bug *timing task* FreeRTOS dan fluktuasi (*jitter*) nilai ADC akibat efek termal pemanas sensor. | 2 | Kode Sumber `main.cpp` & Log Debug |
| Jumat, 25 Sep 2026 | Menangani bug serialisasi payload telemetri dan memutuskan penundaan integrasi fisik LoRa guna mencegah *brownout reset* sebelum rel daya dipisahkan; melanjutkan optimasi kode. | 2 | Commit Git & Skema Daya |
| Sabtu, 26 Sep 2026 | Mengevaluasi hasil uji integrasi modular, mendokumentasikan akar masalah termal sensor, menyusun *corrective action*, serta menyelesaikan Laporan Individu Pekan 5. | 2 | Laporan Individu Pekan 5 |

**Total Jam Kerja Minggu Ini : 14 Jam**

---

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan kelima ini, fokus kerja saya diarahkan pada pengujian integrasi perangkat keras tingkat lanjut serta stabilisasi perangkat lunak:
1. **Pengujian Integrasi Modular Seluruh Sensor:** Bersama Amel (Siti Amalia), saya menghubungkan seluruh sensor target sistem eSOS (Sensor Ultrasonik JSN-SR04T untuk level air tangki, Sensor Gas Amonia MQ-137, Sensor Gas $H_2S$ MQ-136, dan tombol darurat SOS) ke satu papan mikrokontroler ESP32 untuk memvalidasi pembacaan data secara simultan.
2. **Investigasi Kendala Termal Sensor Gas (MQ Series):** Menemukan bahwa modul sensor gas MQ-137 dan MQ-136 mengalami panas fisik yang sangat tinggi pada tabung sensor dan menarik arus pemanas gabungan $\approx 320\text{ mA} - 350\text{ mA}$. Hal ini memicu penurunan tegangan (*voltage sag*) pada rel daya bersama dan fluktuasi referensi ADC.
3. **Debugging Kode Firmware C++:** Membantu Amel menelusuri dan membenahi beberapa *bug* pada firmware:
   - Memperbaiki bug *task starvation* pada FreeRTOS dengan menyetel prioritas `vTaskGasSensors` dan `vTaskUltrasonic` secara proporsional.
   - Mengimplementasikan algoritma *moving average filter* (10 sampel) pada pembacaan analog gas untuk meredam *noise jitter* akibat efek pemanasan sensor.
4. **Keputusan Rekayasa Penundaan Integrasi LoRa:** Memutuskan secara sadar untuk menunda pemasangan modul transceiver LoRa SX1278 fisik pada pekan ini. Karena modul LoRa menarik arus puncak hingga $\approx 120\text{ mA}$ saat transmisi RF, menggabungkannya ke rel daya yang sedang tidak stabil akibat pemanas sensor gas berisiko tinggi memicu *brownout reset* pada ESP32.

**2. Luaran yang Dihasilkan**
- Rangkaian uji integrasi seluruh sensor pada mikrokontroler ESP32.
- Data karakterisasi konsumsi arus pemanas (*heater*) sensor gas MQ-137 dan MQ-136.
- Revisi kode sumber firmware C++ (`main.cpp` & `config.h`) dengan filter *noise* pembacaan ADC dan penjadwalan task FreeRTOS yang lebih stabil.
- Analisis mitigasi sistem catu daya terpisah (*split power rail*) untuk persiapan integrasi LoRa di Pekan 6.

---

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Integrasi seluruh sensor pada satu mikrokontroler | Seluruh sensor tersambung; teridentifikasi kendala termal pemanas | 85% |
| Software (FW) | Pembenahan bug *timing* FreeRTOS & filter ADC | Bug *starvation* & *jitter* analog teratasi, kode stabil | 90% |
| Software (Web) | Kesiapan menerima data ingest integrasi riil | Backend Go & Web SCADA siap (terverifikasi pekan lalu) | 100% |
| Mekanik | Uji tata letak fisik modul terhadap panas | Evaluasi ventilasi udara untuk sensor gas pada casing | 70% |
| Pengujian | Uji fungsional simultan sensor lokal | Pengujian lokal selesai; uji RF LoRa ditunda | 75% |
| Dokumentasi | Pencatatan kendala termal & logbook W5 | Logbook W5 selesai & *Risk Register* diperbarui | 100% |

**Ringkasan Kemajuan**
Meskipun integrasi komunikasi nirkabel LoRa fisik belum dapat diaktifkan pekan ini, pengujian integrasi seluruh sensor secara modular berhasil membongkar anomali kritis berupa beban termal dan lonjakan arus kumparan pemanas (*heater coil*) sensor gas MQ. Penemuan dini ini mencegah kegagalan sistem fatal (*brownout crash*) sebelum alat dideploy ke lapangan. Proses *debugging* kode firmware bersama Amel membuahkan kerangka program yang jauh lebih stabil dan tahan terhadap gangguan derau listrik.

---

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | **Sensor Gas Mengalami Panas Tinggi (*Overheating Heater Coil*):** Modul MQ-137 dan MQ-136 memanas secara fisik ($>50^\circ\text{C}$) dan menarik arus kontinu hingga $\approx 350\text{ mA}$ untuk elemen pemanas internal $\text{SnO}_2$. | Suhu bodi sensor sangat panas, membebani rel catu daya 5V, memicu *voltage sag*, dan menimbulkan derau pembacaan ADC pada ESP32. | **Tinggi** |
| 2 | **Penundaan Integrasi Modul LoRa Fisik:** Integrasi modul komunikasi radio LoRa belum dapat digabungkan dengan rangkaian sensor penuh. | Pengujian transmisi nirkabel jarak jauh tertunda demi memprioritaskan stabilitas kelistrikan dan keselamatan komponen. | **Sedang** |
| 3 | **Fluktuasi Nilai ADC (*Noise Jitter*):** Nilai konversi analog-ke-digital dari sensor gas berosilasi liar saat pemanas aktif. | Data telemetri konsentrasi gas (ppm) tidak akurat dan berpotensi memicu alarm palsu (*false positive incident alert*). | **Sedang (Terselesaikan)** |

**Analisis Penyebab**
1. **Karakteristik Fisik Sensor MQ:** Sensor gas semikonduktor oksida logam ($\text{SnO}_2$) memerlukan suhu operasional tinggi ($200^\circ\text{C}-300^\circ\text{C}$ pada keping mikro internal) yang dipanaskan oleh kumparan pemanas $5\text{V}$ dengan daya sekitar $800-900\text{ mW}$ per unit. Ketika dua sensor dinyalakan bersamaan pada papan *breadboard* menggunakan sumber daya USB tunggal, regulator daya bekerja pada batas kapasitas maksimumnya sehingga bodi sensor terasa sangat panas dan tegangan suplai mengalami *sagging*.
2. **Ketergantungan Daya LoRa:** Modul radio SX1278 membutuhkan tegangan $3.3\text{V}$ yang sangat stabil dengan kemampuan memasok arus sesaat (*burst current*) hingga $120\text{ mA}$ saat paket dipancarkan. Mengaktifkan LoRa pada kondisi daya yang sedang mengalami *sagging* akibat sensor gas dipastikan akan memicu *brownout reset* pada ESP32.

---

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Panas Berlebih & Beban Arus Sensor Gas MQ | Mengimplementasikan arsitektur daya terpisah (*Split Power Rail*): elemen pemanas sensor gas ditenagai langsung dari jalur konverter buck/baterai eksternal (dengan heatsink/ventilasi isolasi), terpisah dari jalur logika $3.3\text{V}$ ESP32. | Daffa H. & Ilman | Pertengahan Pekan 6 |
| Penundaan Integrasi LoRa | Mematangkan seluruh *driver* sensor dan penanganan error terlebih dahulu. Integrasi LoRa fisik dijadwalkan langsung setelah rel daya terpisah terpasang dan teruji stabil. | Daffa H. & Siti Amalia | Akhir Pekan 6 |
| Fluktuasi (*Jitter*) Pembacaan ADC Gas | Menerapkan algoritma *Oversampling & Moving Average Filter* (10 siklus baca per detik) pada fungsi `vTaskGasSensors` di firmware untuk menghaluskan fluktuasi tegangan sesaat. | Siti Amalia & Daffa H. | **Selesai (Pekan 5)** |

---

### H. Dokumentasi Kemajuan
Berikut adalah bukti kegiatan dan pengerjaan teknis pada pekan kelima:

> **[BUKTI 1: PENGUJIAN INTEGRASI SENSOR & INVESTIGASI SENSOR PANAS]**  
> *Foto sesi pengujian integrasi seluruh sensor (ultrasonik, MQ-137, MQ-136, servo) di meja laboratorium bersama Amel, memperlihatkan pengukuran kelistrikan dan kondisi fisik sensor yang memanas.*  
> `![Pengujian Integrasi Sensor](../media/w5_pengujian_sensor_panas.jpg)`

> **[BUKTI 2: SESI DEBUGGING KODE FIRMWARE BERSAMA AMEL]**  
> *Tangkapan layar lingkungan pengembangan PlatformIO / VS Code yang memperlihatkan proses pelacakan bug, penyesuaian filter moving average ADC, dan optimasi task FreeRTOS pada file `main.cpp`.*  
> `![Debugging Firmware Bersama Amel](../media/w5_debugging_firmware_amel.png)`

> **[BUKTI 3: REKAPITULASI PROGRES & LOGBOOK SHEETS W5]**  
> *Tangkapan layar pembaruan berkas pelacak progres proyek mingguan dan lembar catatan pengujian sistem Pekan 5.*  
> `![Rekapitulasi Logbook Pekan 5](../media/w5_logbook_sheets_pekan5.png)`

---

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Kolaborasi Intensif Bersama Amel (Siti Amalia):** Melakukan *pair programming* dan *hardware debugging* secara luring untuk menyelesaikan kendala pembacaan sensor dan penataan antrean FreeRTOS.
- **Konsultasi Jalur Daya Bersama Ilman:** Mendiskusikan modifikasi jalur pengkabelan kelistrikan agar modul baterai 18650 dan konverter buck mampu memasok arus pemanas sensor gas tanpa mengganggu mikrokontroler.
- **Sinkronisasi Desain Mekanik Bersama Darrell:** Memberikan masukan teknis mengenai perlunya lubang ventilasi sirkulasi udara pada kompartemen sensor gas di casing 3D untuk membuang panas akumulatif pemanas.

**Kontribusi Pribadi**
Mengidentifikasi risiko kegagalan sistem (*brownout risk*) akibat anomali termal sensor gas, mengambil keputusan strategis penundaan LoRa demi keselamatan perangkat keras, serta berkontribusi langsung membenahi algoritma penyaringan derau (*noise filtering*) pada firmware C++.

---

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Kerja sama tim yang sangat solid bersama Amel saat melakukan *troubleshooting* langsung di lab.
- Kemampuan mendeteksi anomali konsumsi daya dan panas sensor sebelum menyebabkan kerusakan permanen pada modul mikrokontroler.
- Kode firmware menjadi jauh lebih tangguh (*robust*) berkat implementasi filter rata-rata bergerak dan manajemen prioritas task.

**Yang Perlu Diperbaiki**
- Perlu lebih teliti membaca *datasheet* komponen sejak awal, khususnya mengenai kurva kebutuhan daya pemanas (*heater rating*) sensor seri MQ.
- Pembagian jalur daya pada tahap perakitan *breadboard* seharusnya sudah dipisahkan antara beban daya tinggi (*heater/servo*) dan beban sinyal digital ($3.3\text{V}$).

**Pelajaran yang Didapat Minggu Ini**
- *Safety and electrical stability come first before wireless transmission.* Mengintegrasikan radio pemancar tanpa memastikan stabilitas catu daya hanya akan menghasilkan *intermittent bugs* yang sulit dilacak.
- Panas pada sensor gas MQ adalah sifat bawaan pemanas keramik katalitik, namun manajemen termal dan suplai dayanya harus dirancang secara terisolasi.

---

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Pemasangan rel catu daya terpisah (*Split Power Rail*) untuk suplai pemanas sensor gas bersama Ilman. | Suplai tegangan stabil 5V (heater) dan 3.3V (ESP32) tanpa *voltage sag*. | 4 Jam |
| 2 | Eksekusi integrasi modul radio LoRa fisik (SX1278) setelah jalur daya terbukti stabil. | Paket telemetri lokal berhasil dipancarkan via frekuensi LoRa 915 MHz. | 5 Jam |
| 3 | Uji transmisi nirkabel LoRa Node ke Gateway dan penerusan ke Web SCADA EMS posko. | Data riil dari seluruh sensor tertampil secara live di dashboard web. | 5 Jam |

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
- "Bagus. Menemukan kendala pemanas sensor gas sejak dini adalah langkah mitigasi yang tepat. Pastikan pemisahan jalur daya diselesaikan sebelum modul LoRa dinyalakan agar tidak terjadi restart mendadak."
