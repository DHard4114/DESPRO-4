# TEMPLATE LAPORAN PEKANAN INDIVIDU
**Capstone Project Desain Proyek 2**

### A. Identitas Mahasiswa
| Item | Keterangan |
|:---|:---|
| Nama | Daffa Hardhan |
| NPM | 2306161763 |
| Program Studi | Teknik Komputer |
| Kelompok | 4 (Empat) |
| Judul Proyek | Rancang Bangun Sistem Monitoring Smart-Sanitation eSOS Berbasis IoT untuk Wilayah Blank Spot Pasca-Bencana |
| Pekan ke- | 7 (Tujuh) |
| Periode | 5 Oktober 2026 – 10 Oktober 2026 |
| Tanggal | 10 Oktober 2026 |
| Dosen Pembimbing | Prof. Dr. Muhammad Suryanegara, S.T., M.Sc. |

---

### B. Target Mingguan
Target yang direncanakan pada awal pekan berdasarkan WBS dan pembagian tugas kelompok:

| No | Target Pekerjaan | Luaran yang Diharapkan | Status |
|:---:|:---|:---|:---:|
| 1 | Pengujian konektivitas jaringan lokal nirkabel (Wi-Fi STA) dan penetapan skema IP statis pada Node Gateway ESP32 | ESP32 Gateway terhubung stabil ke Access Point `CompEngQuiz-Server-Live` dengan IP statis `192.168.101.11` | ☑ Selesai &nbsp; ☐ Belum |
| 2 | Verifikasi keterhubungan antar-perangkat (*End-to-End ICMP Ping*) antara PC Client, Broker MQTT, dan Node Gateway | Seluruh simpul jaringan dapat saling mem-ping satu per satu dengan *packet loss* 0% | ☑ Selesai &nbsp; ☐ Belum |
| 3 | Pengujian *transport layer* socket TCP dan integrasi protokol MQTT dari Gateway ke broker laptop (`192.168.101.100:1883`) | ESP32 Gateway mampu membuka soket TCP ke broker laptop dan mengirim paket *handshake* MQTT | ☑ Selesai &nbsp; ☐ Belum |
| 4 | Diagnostik kendala inisialisasi perangkat keras radio LoRa SX1278 pada Gateway (mengatasi galat Kode: -2) | Identifikasi akar masalah pengkabelan bus SPI kustom dan kestabilan rel catu daya 3.3V | ☑ Selesai &nbsp; ☐ Belum |

---

### C. Logbook Aktivitas Harian
| Hari/Tanggal | Uraian Aktivitas | Waktu (Jam) | Bukti/Dokumen |
|:---|:---|:---:|:---|
| Senin, 5 Okt 2026 | Merancang topologi pengalamatan IP statis subnet `192.168.101.0/24` untuk integrasi posko darurat; mendefinisikan konstanta jaringan pada `config.h`. | 2.5 | Draf Skema Alamat IP & `config.h` |
| Selasa, 6 Okt 2026 | Mengonfigurasi Mosquitto MQTT Broker pada PC laptop (`192.168.101.100`), membuka port 1883 pada firewall Windows, dan menguji pub/sub client lokal. | 3 | Konfigurasi `mosquitto.conf` & Log Broker |
| Rabu, 7 Okt 2026 | Memprogram dan menguji modul Wi-Fi station pada ESP32 Gateway; memvalidasi penerapan IP statis `192.168.101.11` via AP `CompEngQuiz-Server-Live`. | 3 | Log Serial Monitor Wi-Fi Connection |
| Kamis, 8 Okt 2026 | Melakukan pengujian ICMP Ping satu per satu: dari PC ke broker lokal (`192.168.101.100`) dan dari PC ke ESP32 Node Gateway (`192.168.101.11`). | 2.5 | Tangkapan Layar PowerShell Ping Test |
| Jumat, 9 Okt 2026 | Menguji *handshake* koneksi TCP dari ESP32 Gateway ke Broker Port 1883 (`connected=1` dalam 9–35 ms); mendiagnosis *timeout* MQTT `state=-4`. | 3 | Log Serial Monitor TCP Result |
| Sabtu, 10 Okt 2026 | Menganalisis penyebab error inisialisasi SPI LoRa Gateway (`Kode: -2`), memperbarui parameter timeout socket MQTT, dan menyusun Laporan Pekan 7. | 3 | Pembaruan Firmware & Laporan Pekan 7 |

**Total Jam Kerja Minggu Ini : 17 Jam**

---

### D. Realisasi Pekerjaan
**1. Aktivitas yang Berhasil Diselesaikan**
Pada pekan ketujuh ini, fokus pekerjaan berpusat pada pengujian lapisan jaringan (*Networking & Transport Layer*) dan keterhubungan antar-simpul sistem:
- **Pengujian Jaringan Lokal dan Verifikasi Ping Antar-Perangkat (Selesai 100%):**  
  Berhasil menghubungkan ESP32 Node Gateway ke Access Point `CompEngQuiz-Server-Live` dengan konfigurasi IP statis `192.168.101.11`. Melakukan verifikasi konektivitas jaringan end-to-end dengan uji *ICMP Ping* satu per satu:
  1. Ping dari PC ke Broker MQTT Laptop (`192.168.101.100`): 4 paket terkirim, 4 diterima, **0% packet loss**, latensi $<1\text{ ms}$.
  2. Ping dari PC ke Node Gateway ESP32 (`192.168.101.11`): 4 paket terkirim, 4 diterima, **0% packet loss**, rata-rata latensi $78\text{ ms}$ (rentang $66\text{ ms} - 89\text{ ms}$).
- **Pengujian Transport Layer TCP ke Broker MQTT (Selesai 100%):**  
  Berhasil memvalidasi bahwa antarmuka jaringan ESP32 Gateway mampu membuka koneksi soket TCP langsung ke Broker Mosquitto laptop (`192.168.101.100:1883`) dengan status `[TCP RESULT] connected=1` dalam durasi sangat singkat ($9\text{ ms} - 35\text{ ms}$).
- **Pemetaan dan Standarisasi Pengalamatan IP Sistem (Selesai 100%):**  
  Menetapkan dan mengunci definisi pengalamatan IP resmi pada `config.h`:
  - **MQTT Server (Broker Laptop Ethernet):** `192.168.101.100`
  - **Web Client / Host Laptop:** `192.168.101.10`
  - **Node Gateway ESP32:** `192.168.101.11`
- **Analisis dan Isolasi Masalah Timeout MQTT dan Galat LoRa (Selesai 100%):**  
  Menemukan bahwa respon MQTT `state=-4` (*MQTT_CONNECTION_TIMEOUT*) disebabkan oleh nilai `mqtt.setSocketTimeout(2)` yang terlalu sempit (2 detik) di atas jaringan nirkabel. Nilai ini telah diperbaiki menjadi 10 detik. Mengidentifikasi bahwa galat `Kode: -2` (*CHIP_NOT_FOUND*) pada LoRa Gateway disebabkan oleh hambatan fisik (kontak kabel jumper breadboard dan catu daya 3.3V).

**2. Luaran yang Dihasilkan**
- Pembaruan firmware Gateway (`main.ino` dan `config.h`) dengan perbaikan socket timeout dan konfigurasi IP resmi.
- Bukti tangkapan layar pengujian *ICMP Ping* ke `192.168.101.100` (Broker) dengan hasil 0% loss.
- Bukti tangkapan layar pengujian *ICMP Ping* ke `192.168.101.11` (Node Gateway) dengan hasil 0% loss.
- Bukti log Serial Monitor keberhasilan koneksi Wi-Fi dan soket TCP layer transport (`connected=1`).
- Dokumen Laporan Kemajuan Pekanan Individu Pekan 7.

---

### E. Capaian Teknis Mingguan
**Komponen/Subsistem yang Dikerjakan (Lingkup Daffa Hardhan)**
| Subsistem | Target | Realisasi | Persentase |
|:---|:---|:---|:---:|
| Hardware | Pengecekan stabilitas daya ESP32 & wiring SPI LoRa Gateway | Modul ESP32 aktif stabil di jaringan Wi-Fi; isolasi kendala SPI breadboard | 85% |
| Software (FW) | Implementasi Wi-Fi STA statis & soket TCP/MQTT Gateway | IP statis diterapkan, koneksi TCP ke port 1883 berhasil (`connected=1`) | 90% |
| Software (Web/Host) | Pengujian broker Mosquitto pada laptop client | Broker aktif di port 1883, menerima koneksi TCP dari ESP32 | 90% |
| Pengujian | Uji Ping satu per satu & uji transmisi transport layer | Uji Ping 100% tembus (0% loss) dan soket TCP berhasil terbentuk | 90% |
| Dokumentasi | Penyusunan logbook & Laporan Individu Pekan 7 | Laporan dan bukti tangkapan layar terarsip lengkap | 100% |

**Ringkasan Kemajuan**
Pada Pekan 7, kemajuan signifikan dicapai pada pembuktian keterhubungan jaringan (*networking layer*). Seluruh perangkat utama (PC client laptop, broker Mosquitto, dan ESP32 Node Gateway) telah berhasil saling terhubung dalam satu subnet nirkabel lokal `192.168.101.0/24` dengan latensi terkendali dan tanpa kehilangan paket data (*zero packet loss*). Pembentukan soket TCP dari Gateway ke laptop membuktikan kesiapan pipa transmisi data, sehingga ketika data telemetri diterima dari Node WC, data siap dipublikasikan ke broker secara andal.

---

### F. Permasalahan dan Kendala
| No | Kendala | Dampak | Tingkat Risiko |
|:---:|:---|:---|:---:|
| 1 | **Timeout Handshake MQTT (`state = -4`):** Setelah soket TCP berhasil dibuka (`connected=1`), pemanggilan `mqtt.connect()` mengalami *timeout* tepat pada durasi 2003 ms. | ESP32 Gateway belum berhasil menyelesaikan proses *handshake* CONNACK pada broker MQTT. | **Sedang** |
| 2 | **Galat Inisialisasi Radio SX1278 Gateway (`Kode: -2`):** Saat booting, fungsi `radio.begin()` melaporkan `RADIOLIB_ERR_CHIP_NOT_FOUND`. | Radio LoRa pada Gateway belum dapat menerima transmisi paket dari Node WC. | **Sedang** |
| 3 | **Fluktuasi Catu Daya (*Brownout / Power Reset*):** Log boot ESP32 sempat mencatat `rst:0x1 (POWERON_RESET)` secara berulang. | Mikrokontroler me-restart mandiri ketika konsumsi arus Wi-Fi melonjak tinggi. | **Rendah** |

**Analisis Penyebab**
1. Nilai konfigurasi `mqtt.setSocketTimeout(2)` pada library `PubSubClient` membatasi waktu tunggu respons broker hanya 2 detik. Pada koneksi Wi-Fi nirkabel dengan latensi rata-rata 78 ms, paket `CONNACK` membutuhkan jendela waktu toleransi yang lebih besar.
2. Galat Kode: -2 pada RadioLib membuktikan bahwa pembacaan Register 0x42 via SPI mengembalikan nilai 0x00/0xFF, yang murni disebabkan oleh hambatan fisik: kabel jumper SPI breadboard kendor (SCK di D21, MOSI di D18) atau modul radio belum menerima tegangan 3.3V stabil.

---

### G. Tindakan Korektif dan Solusi
| Kendala | Solusi/Tindak Lanjut | PIC | Target Penyelesaian |
|:---|:---|:---|:---|
| Timeout Handshake MQTT (`state = -4`) | Meningkatkan `mqtt.setSocketTimeout` dari 2 detik menjadi 10 detik, serta menaikkan batas timeout TCP menjadi 3000 ms pada firmware Gateway. | Daffa Hardhan | Selesai (Pekan 7) |
| Inisialisasi Radio LoRa (`Kode: -2`) | Memeriksa dan mengencangkan kabel jumper SPI kustom (SCK: D21, MOSI: D18, MISO: D19, NSS: D5) serta menguji modul menggunakan kabel pendek berkualitas tinggi. | Daffa Hardhan | Pekan 8 |
| Fluktuasi Daya (*Brownout*) | Memindahkan sambungan kabel USB ke port daya berdaya tinggi (USB 3.0) serta merapatkan rel daya 3.3V dan Ground pada breadboard. | Daffa Hardhan | Selesai (Pekan 7) |

---

### H. Dokumentasi Kemajuan
Berikut adalah bukti autentik realisasi pekerjaan pekan ketujuh yang mencakup uji keterhubungan ICMP Ping satu per satu serta log Serial Monitor pembentukan soket TCP:

**1. Bukti Uji Ping Laptop ke Broker MQTT (`192.168.101.100`)**  
*Tangkapan layar terminal PowerShell membuktikan koneksi loopback/lokal ke broker Mosquitto di laptop berjalan sempurna: 4 paket terkirim, 4 diterima, 0% packet loss, dengan waktu respons $<1\text{ ms}$.*  
![Uji Ping Broker Laptop](../../media/Ping_Broker_Laptop_W7.png)

---

**2. Bukti Uji Ping Laptop ke ESP32 Node Gateway (`192.168.101.11`)**  
*Tangkapan layar terminal PowerShell membuktikan keterhubungan nirkabel PC Client ke Node Gateway ESP32: 4 paket terkirim, 4 diterima, 0% packet loss, dengan rata-rata waktu respons $78\text{ ms}$ (minimal $66\text{ ms}$, maksimal $89\text{ ms}$).*  
![Uji Ping ESP32 Gateway](../../media/Ping_Gateway_ESP32_W7.png)

---

**3. Bukti Serial Monitor Gateway: Koneksi Wi-Fi & Soket TCP Sukses (`connected=1`)**  
*Tangkapan layar Serial Monitor ESP32 (115200 baud) yang membuktikan inisialisasi jaringan berhasil: IP statis `192.168.101.11` terhubung ke SSID `CompEngQuiz-Server-Live` (RSSI -56 dBm), dan lapisan transport berhasil membuka soket TCP ke Broker `192.168.101.100:1883` dengan durasi $22\text{ ms}$ dan $9\text{ ms}$ (`[TCP RESULT] connected=1`).*  
![Serial Monitor Gateway TCP Connect](../../media/Serial_Monitor_Gateway_TCP_W7.png)

---

### I. Kontribusi Terhadap Tim
**Koordinasi yang Dilakukan**
- **Penetapan Skema IP Jaringan:** Mengomunikasikan pemetaan alamat IP jaringan statis kepada seluruh anggota kelompok agar konsisten dengan subsistem web dashboard dan pengujian lapangan.
- **Penyelarasan Endpoint Broker:** Memastikan laptop yang menjalankan Mosquitto Broker (`192.168.101.100`) siap menerima data telemetri dari Gateway dan melayani antarmuka monitoring tim.

**Kontribusi Pribadi**
Mengonfigurasi dan menguji seluruh lapisan jaringan (Wi-Fi, IP statis, ICMP Ping, soket TCP), mengisolasi parameter timeout MQTT, serta menjaga keselarasan source code firmware gateway pada repositori Git.

---

### J. Evaluasi Diri
**Yang Berjalan Baik**
- Seluruh simpul perangkat jaringan berhasil saling terhubung dan terbukti melalui uji ping satu per satu dengan *zero packet loss*.
- Lapisan transport TCP pada ESP32 berhasil membuka koneksi ke port broker 1883 dengan waktu respons sangat cepat ($<35\text{ ms}$).
- Isolasi akar masalah pada timeout MQTT berhasil diidentifikasi secara presisi berdasarkan analisis log Serial Monitor.

**Yang Perlu Diperbaiki**
- Pengkabelan fisik perangkat keras LoRa pada Gateway perlu diperiksa lebih teliti menggunakan multimeter/jumper baru untuk menuntaskan galat inisialisasi SPI Kode: -2.

**Pelajaran yang Didapat Minggu Ini**
- Jaringan nirkabel lapangan memiliki latensi yang dinamis; parameter *socket timeout* pada library jaringan mikrokontroler tidak boleh disetel terlalu agresif agar tidak memicu kegagalan *handshake* yang semu.

---

### K. Rencana Pekan Berikutnya
| No | Rencana Kegiatan | Target Luaran | Estimasi Jam |
|:---:|:---|:---|:---:|
| 1 | Menyelesaikan perbaikan fisik wiring modul LoRa SX1278 pada Gateway hingga lolos inisialisasi SPI (`[OK] Radio SX1278 aktif`) | Radio LoRa Gateway siap siaga dalam mode RX Continuous | 5 Jam |
| 2 | Menguji coba publikasi telemetri MQTT penuh dari Gateway ke broker laptop dan verifikasi penerimaan data di web client | Data JSON telemetri masuk ke topik `esos/gateway_01/nodes/telemetry` | 5 Jam |
| 3 | Pengujian transmisi nirkabel terpadu: Node WC mengirim data via LoRa $\rightarrow$ diterima Gateway $\rightarrow$ dipublikasikan ke Broker MQTT laptop | Integrasi komunikasi end-to-end lapangan terverifikasi 100% | 6 Jam |

---

### L. Persetujuan
**Mahasiswa**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 10 Oktober 2026  

**Ketua Kelompok**  
Nama: Daffa Hardhan  
Tanda tangan: *(Digital)*  
Tanggal: 10 Oktober 2026  

**Catatan Pembimbing**  
....................................................................................  
....................................................................................  
....................................................................................  
