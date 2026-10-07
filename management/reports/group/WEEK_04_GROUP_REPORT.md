# LAPORAN KEMAJUAN PEKANAN
## DESAIN PROYEK TEKNIK ELEKTRO, KOMPUTER, BIOMEDIK 2
### (LAPORAN INTEGRASI SISTEM VERSI 1)

**Judul Proyek** : RANCANG BANGUN SISTEM MONITORING SMART-SANITATION ESOS BERBASIS IOT UNTUK WILAYAH BLANK SPOT PASCA-BENCANA  
**Kelompok** : 4 (Empat)  
**Pekan Ke-** : 4 (Empat)  
**Periode** : 14 September 2026 – 19 September 2026  
**Dosen Pembimbing** : Prof. Dr. Muhammad Suryanegara, S.T., M.Sc.  
**Ketua Kelompok** : Daffa Hardhan (2306161763)  
**Anggota Kelompok** :  
1. Raka Arrayan Muttaqien (2306161800)  
2. Siti Amalia Nurfaidah (2306161851)  
3. Darrell Alfath (2306266810)  
4. Muhammad Ilman Zuhriy (2306266786)  

---

### 1. Ringkasan Kemajuan Pekanan
Pada pekan keempat ini, tim berfokus pada pemenuhan **PMK Pekan 4: "Mahasiswa mampu mengintegrasikan subsistem dan memverifikasi fungsi dasar sistem secara keseluruhan"**. Tim berhasil merealisasikan **Integrasi Sistem Versi 1** yang menghubungkan subsistem perangkat keras dasar, firmware I/O, server ETL Go, basis data PostgreSQL 17, hingga antarmuka Web Dashboard SCADA (EMS) mandiri secara *real-time* via WebSocket. 

Pengujian aliran data *end-to-end* berhasil dibuktikan dengan latensi rendah (~15 ms) menggunakan simulasi generator telemetri ber-QoS 1. Dari sisi perangkat keras, perakitan kabel kelistrikan (*wiring harness*), modul daya baterai LiPo, dan sensor-aktuator dasar telah dirangkai pada *breadboard*, serta *base code* firmware C++ ESP32 telah diselaraskan dengan arsitektur antrean FreeRTOS. Seluruh kendala teknis prioritas telah diidentifikasi dan ditindaklanjuti dalam daftar *corrective action* untuk pekan 5–7.

---

### 2. Target Pekan Ini
| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Integrasi Perangkat Lunak: Pembangunan Web Dashboard SCADA (EMS) dengan WebSocket & Chart.js live | Dashboard SCADA mandiri full-width menampilkan telemetri & kendali aktuator | [x] Tercapai &nbsp; [ ] Belum |
| 2 | Integrasi Perangkat Keras & Firmware: *Wiring* kelistrikan dasar dan pembuatan *Base Code* I/O ESP32 | Rangkaian jumper terhubung & kerangka C++ sensor-aktuator siap uji | [x] Tercapai &nbsp; [ ] Belum |
| 3 | Pengujian Komunikasi & Aliran Data End-to-End | Verifikasi transmisi data sensor, deduplikasi QoS 1, dan *downlink command* | [x] Tercapai &nbsp; [ ] Belum |
| 4 | Manajemen Proyek: Evaluasi WBS, *Risk Register*, dan Rekapitulasi Jam Kerja Sheets W4 | Logbook harian terkonsolidasi & identifikasi gap performa | [x] Tercapai &nbsp; [ ] Belum |

---

### 3. Realisasi Kegiatan
| No | Kegiatan | Penanggung Jawab | Hasil |
|:---:|:---|:---|:---|
| 1 | Pengembangan antarmuka Web Dashboard SCADA (EMS) mandiri berstandar AMR-RS UI. | Daffa Hardhan | Dashboard SCADA live di port `:8000` dengan arsitektur *fluid canvas*, 4 kartu KPI, 2 grafik dinamis, dan tabel audit stream. |
| 2 | Integrasi WebSocket Hub pada Server Go dan pengujian deduplikasi QoS 1. | Daffa Hardhan | Aliran data dari broker MQTT diteruskan ke browser secara *non-blocking* dengan latensi transmisi rata-rata 14,8 ms. |
| 3 | Perakitan kelistrikan fisik (*wiring harness*) modul sensor analog, servo, dan catu daya baterai. | M. Ilman Zuhriy & Daffa Hardhan | Breadboard pengujian terpasang rapi dengan jalur VCC/GND terisolasi untuk menghindari *voltage drop*. |
| 4 | Penulisan kerangka kode sumber (*base code*) firmware C++ ESP32 FreeRTOS. | Siti Amalia N. & Daffa Hardhan | `main.cpp` mengimplementasikan *FreeRTOS Queue*, pembacaan sensor ultrasonik `NewPing`, interupsi tombol SOS (`isr_sos_button`), dan kendali servo `ESP32Servo`. |
| 5 | Penyetelan (*tuning*) parameter modul radio LoRa SX1278 (frekuensi 915 MHz, SF7, BW 125 kHz). | Siti Amalia N. | Pengujian awal komunikasi point-to-point pada jarak dekat di lab. |
| 6 | Pemodelan 3D Enclosure Modular IP54 dan dudukan katup aktuator mekanik pipa PVC. | Darrell Alfath | File CAD 3D preliminary siap untuk simulasi penempatan komponen dan pencetakan prototype. |
| 7 | Pengujian fungsional (*functional & smoke testing*) dan pencatatan metrik performa. | Raka Arrayan M. & Daffa Hardhan | Rangkaian pengujian lolos uji fungsional awal; dokumen *evidence register* diperbarui. |

---

### 4. Kemajuan Tiap Anggota
| Nama | Tugas Utama Pekan Ini | Persentase Penyelesaian |
|:---|:---|:---:|
| **Daffa Hardhan** | Redesain SCADA EMS Web Dashboard, integrasi WebSocket broadcast, penyelesaian bug deduplikasi QoS 1, pendampingan wiring, evaluasi PM logbook. | 100% |
| **M. Ilman Zuhriy** | Perakitan kabel kelistrikan (*wiring harness*), integrasi modul daya baterai LiPo 3.7V, dan uji kontinuitas jalur sensor. | 100% |
| **Siti Amalia N.** | Penyetelan frekuensi radio LoRa SX1278, penyesuaian payload binary C-Struct, dan integrasi modulasi LoRaWAN dasar. | 100% |
| **Darrell Alfath** | Desain mekanik 3D enclosure modular tahan air IP54 dan rancang dudukan mekanis katup servo MG996R. | 100% |
| **Raka Arrayan M.** | Pengujian fungsional sistem, smoke testing endpoint REST API, dokumentasi logbook, dan verifikasi kepatuhan pengujian. | 100% |

---

### 5. Hasil Implementasi Sistem (Versi 1)
#### A. Hardware
- **Komponen yang Direalisasikan:** Mikrokontroler ESP32 DevKit V1, Sensor Gas Amonia MQ-137, Sensor Gas H₂S MQ-136, Sensor Ultrasonik Tahan Air JSN-SR04T, Motor Servo Torsi Tinggi MG996R, Modul Radio LoRa SX1278 (915 MHz), Modul Solar Charger TP4056 dengan Baterai LiPo 1S 3.7V 2000mAh.
- **Status Integrasi:** Subsistem kelistrikan dan interkoneksi I/O berhasil dirangkai pada *breadboard*. Modul siap diintegrasikan dengan modul transmisi LoRa fisik pada sesi lab Pekan 5.

#### B. Software & Arsitektur Data
- **Fitur yang Telah Selesai:**
  1. *ETL Pipeline & Ingestion Engine:* Pemrosesan pesan MQTT berbasis Goroutine worker pool dengan throughput > 1000 pesan/detik.
  2. *Deduplikasi Monotonik QoS 1:* Mekanisme *in-memory caching* untuk membuang paket duplikat hasil transmisi ulang jaringan.
  3. *Downlink Actuation REST API:* Pengiriman instruksi pembukaan katup (90°) dan penutupan katup (0°) dengan enkapsulasi kunci idempotensi UUIDv7.
  4. *SCADA Web Dashboard (EMS):* Antarmuka web responsif full-width mengadopsi standar visual SCADA AMR-RS UI (Makara Navy `#0F1B3D`, Gold `#FFD400`), grafik dinamis Chart.js tanpa tabrakan teks, dan tabel audit streaming.
- **Fitur yang Masih Dikembangkan:** Modul enkripsi payload LoRa end-to-end (AES-128) dan *store-and-forward* LittleFS ring buffer saat koneksi putus.

#### C. Mekanik
- Desain enclosure modular berbahan ABS/PETG dengan kompartemen ganda: ruang basah (katup servo pembilas & sensor ultrasonik tangki) dan ruang kering berproteksi IP54 (ESP32, baterai, modul LoRa).

---

### 6. Dokumentasi Hasil
1. **Gambar 1 – Antarmuka Web Dashboard SCADA (EMS) Real-Time:**  
   *Tangkapan layar antarmuka pemantauan telemetri mandiri di browser (`http://localhost:8000`) yang memperlihatkan 4 KPI cards (Level Air, Baterai, Amonia, H2S), visualisasi grafik dinamis via WebSocket, tombol kendali aktuator katup, dan audit log paket.*  
   ![Dashboard EMS SCADA](../media/Dashboard%20eSOS.png)

2. **Gambar 2 – Dokumentasi Perakitan Kabel (*Wiring*) Perangkat Keras:**  
   *Dokumentasi sesi kerja tim di posko/lab dalam merangkai interkoneksi kelistrikan sensor analog, aktuator servo, dan ESP32 pada breadboard pengujian.*  
   ![Perakitan Wiring Hardware](../media/Bantu%20Wiring.png)

3. **Gambar 3 – Kode Sumber (*Base Code*) Firmware C++ ESP32:**  
   *Kerangka firmware `main.cpp` yang memadukan FreeRTOS queue, sensor sonar ultrasonik, kendali servo pembilas, dan penanganan interupsi tombol darurat SOS.*  
   ![Basecode Firmware C++](../media/Basecode%20Sensor%20dan%20Aktuator.png)

---

### 7. Pengujian yang Dilakukan (Verifikasi PMK Pekan 4)
| Parameter / Skenario Pengujian | Metodologi & Target Desain | Hasil Aktual | Status |
|:---|:---|:---|:---:|
| **Smoke Testing Server Go & Broker MQTT** | Server Go & Mosquitto dapat *booting* dan mengikat port `:8000` & `:1883` tanpa *crash* / *panic*. | Server dan broker aktif secara stabil, memuat *Threshold Cache* ke RAM dalam 12 ms. | [x] Lulus &nbsp; [ ] Tidak |
| **Functional Testing Aliran Data Real-Time** | Telemetri sensor diterima tiap 2 detik, diurai oleh ETL, dan di-*push* via WebSocket ke browser. | Data masuk dengan frekuensi 0,5 Hz tanpa *frame drop*; grafik Chart.js merender data kontinu. | [x] Lulus &nbsp; [ ] Tidak |
| **Deduplikasi Pesan QoS 1** | Paket dengan `SequenceNo` yang sama atau mundur harus di-*drop* untuk mencegah redundansi database. | Paket duplikat berhasil difilter oleh *in-memory LRU cache*; integritas basis data terjaga 100%. | [x] Lulus &nbsp; [ ] Tidak |
| **Downlink Actuator Command Latency** | Instruksi REST API tombol web diteruskan ke server dengan *trace ID* UUIDv7 dalam waktu < 200 ms. | Server merespons `200 OK` dalam rata-rata 18 ms dan mencatat log eksekusi aktuasi. | [x] Lulus &nbsp; [ ] Tidak |
| **Pengujian Konsumsi Daya Awal** | Konsumsi arus mikrokontroler ESP32 pada mode aktif normal tidak melebihi 160 mA. | Terukur rata-rata 112 mA saat Wi-Fi/Radio *active listen* pada tegangan 3,78 V DC. | [x] Lulus &nbsp; [ ] Tidak |

#### Analisis Hasil Pengujian & Gap Performa
Hasil pengujian menunjukkan bahwa **fondasi integrasi perangkat lunak dan aliran data end-to-end telah mencapai 100% kesiapan operasional**. Latensi pemrosesan internal (14,8 ms) jauh melampaui target desain (< 100 ms). Gap performa utama yang diidentifikasi adalah pengujian radio fisik outdoor jarak jauh yang harus ditunda ke Pekan 5 karena modul LoRa fisik sedang melalui proses kalibrasi frekuensi oleh Siti. Gap ini telah berhasil dimitigasi dengan pengujian berbasis simulator telemetri (`dummy_gateway.go`).

---

### 8. Troubleshooting Sistem yang Berhasil Dipecahkan
1. **Kendala Monotonic Sequence Number:** Pada pengujian awal, tabel telemetri tidak memperbarui baris baru karena simulator awal mengirimkan *sequence number* statis (`0`), sehingga ETL menganggap semua pesan berikutnya sebagai duplikat QoS 1.  
   *Solusi:* Mengimplementasikan `atomic.AddUint32` pada generator data simulasi sehingga nomor urut bertambah secara ketat.
2. **Tabrakan Visual Canvas Chart.js (UI Bug):** Penggunaan konfigurasi `usePointStyle` pada Chart.js menyebabkan lingkaran warna legenda menimpa huruf pertama teks label pada monitor beresolusi tinggi.  
   *Solusi:* Menonaktifkan legenda bawaan kanvas dan membangun elemen legenda berbasis HTML flexbox badges Tailwind CSS dengan jarak `gap-6` dan `gap-2` yang sangat lega.
3. **Inkonsistensi Casing JSON Payload:** Perbedaan serialisasi antara snake_case pada REST API dan PascalCase pada struct Go menyebabkan beberapa nilai terbaca `undefined`.  
   *Solusi:* Menambahkan normalisasi fleksibel pada *frontend parser* JavaScript untuk mendukung kedua konvensi penamaan secara otomatis.

---

### 9. Deviasi Terhadap Jadwal
| Aktivitas | Jadwal Awal (WBS) | Realisasi | Keterangan & Analisis Deviasi |
|:---|:---|:---|:---|
| Integrasi Perangkat Lunak & SCADA UI | Pekan 4 | Pekan 4 | **Tepat Waktu** (Dashboard SCADA selesai 100% mengadopsi AMR-RS UI). |
| Perakitan Hardware & Wiring Dasar | Pekan 4 | Pekan 4 | **Tepat Waktu** (Kabel jumper dan power terpasang). |
| Uji Propagasi Radio LoRa Outdoor | Pekan 4 | Pekan 5 | **Tertunda 1 Pekan** (Modul sedang dikalibrasi; digantikan dengan uji simulator end-to-end). |

---

### 10. Daftar Tindakan Korektif (Corrective Actions) Pekan 5 – 7
Sesuai penugasan PMK Pekan 4, berikut adalah daftar rencana tindakan perbaikan terstruktur:

| No | Prioritas Masalah | Tindakan Korektif (Corrective Action) | Alokasi Waktu | PIC |
|:---:|:---|:---|:---:|:---|
| 1 | **Tinggi** | **Uji Komunikasi Nirkabel Lapangan (Outdoor RF):** Menggabungkan modul LoRa fisik dengan rangkaian sensor ESP32 dan melakukan uji transmisi point-to-point menuju Gateway di Posko FTUI. | Pekan 5 | Siti & Daffa |
| 2 | **Tinggi** | **Verifikasi Mekanisme Aktuator Katup Riil:** Menghubungkan output PWM ESP32 ke motor servo MG996R pada pipa drainase sesungguhnya untuk memvalidasi torsi pembilasan. | Pekan 5 | Ilman & Darrell |
| 3 | **Sedang** | **Optimasi Efisiensi Daya (Power Saving):** Menerapkan mode *FreeRTOS Light/Deep Sleep* saat idle untuk memangkas konsumsi arus rata-rata dari ~110 mA ke < 15 mA. | Pekan 6 | Siti & Ilman |
| 4 | **Sedang** | **Kalibrasi Kurva Sensor Gas:** Mengukur nilai $R_0$ pada sensor MQ-137 dan MQ-136 di udara bersih lab untuk memastikan akurasi konversi nilai ADC ke satuan ppm. | Pekan 6 | Raka & Daffa |
| 5 | **Rendah** | **Ketahanan Jaringan (Store-and-Forward):** Mengimplementasikan *LittleFS ring buffer* pada flash ESP32 untuk menampung paket saat sinyal LoRa terputus dan melakukan *flush* otomatis saat koneksi pulih. | Pekan 7 | Daffa & Siti |

---

### 11. Persentase Progress Proyek
| Komponen | Bobot (%) | Progress Pekan Lalu | Progress Pekan Ini (%) | Kontribusi (%) |
|:---|:---:|:---:|:---:|:---:|
| Perancangan | 25% | 90% | **95%** | 23.75% |
| Implementasi Hardware | 25% | 10% | **40%** | 10.00% |
| Implementasi Software | 20% | 45% | **75%** | 15.00% |
| Integrasi Sistem | 15% | 0% | **50%** | 7.50% |
| Pengujian | 5% | 5% | **35%** | 1.75% |
| Dokumentasi | 10% | 70% | **80%** | 8.00% |
| **Progress Total Proyek** | **100%** | **41.25%** | | **66.00%** |

*Catatan Perhitungan:*
Progress proyek melonjak signifikan dari 41.25% ke **66.00%** didorong oleh keberhasilan integrasi perangkat lunak (SCADA Dashboard, WebSocket, ETL Pipeline), perakitan awal kabel perangkat keras, serta penuntasan pengujian fungsional dan *troubleshooting* sistem.

---

### 12. Kesimpulan
Pada pekan keempat ini, Kelompok 4 telah **berhasil memenuhi seluruh capaian PMK Pekan 4**. Purwarupa awal sistem telah berfungsi, integrasi perangkat lunak dari basis data hingga Web Dashboard SCADA telah terverifikasi secara *end-to-end*, dan perakitan kabel perangkat keras dasar telah terealisasi. Seluruh kendala teknis prioritas telah teridentifikasi dan dipetakan ke dalam rencana tindakan korektif untuk pekan 5–7. Tim siap melangkah ke tahap pengujian lapangan terpadu dan demonstrasi purwarupa untuk Evaluasi Tengah Semester (*Phase Gate 3*).

---

### Persetujuan
| Jabatan | Nama | Tanda Tangan | Tanggal |
|:---|:---|:---:|:---:|
| **Ketua Kelompok** | Daffa Hardhan (NPM 2306161763) | *(Digital Signature)* | 19 September 2026 |
| **Dosen Pembimbing** | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. | ................................... | ......................... |
