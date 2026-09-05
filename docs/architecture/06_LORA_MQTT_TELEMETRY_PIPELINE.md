# DOKUMEN ARSITEKTUR TELEMETRI LORA & MQTT (STAR TOPOLOGY)
## Smart-Sanitation eSOS - Jaringan Terisolasi (Intranet Blank Spot)

Dokumen ini mendefinisikan secara resmi arsitektur komunikasi data antara perangkat *Hardware* di lapangan (Septic Tank) hingga ke Layar Pemantauan (*Dashboard*) di Posko Pusat. 

Sistem ini beroperasi **100% Offline (Intranet)** menggunakan *Router* TP-Link CPE220 dan protokol ringan MQTT, tanpa memerlukan koneksi internet (WWW).

---

## 1. Topologi Jaringan: "Star" (Point-to-Multipoint)
Sistem ini secara tegas **TIDAK menggunakan topologi Mesh**. Untuk memastikan keandalan tingkat tinggi (*high reliability*) dan kemudahan pemrograman, sistem menggunakan topologi **Star**.

*   **Mengapa Bintang (Star)?** Jangkauan frekuensi LoRa 433MHz sangat jauh (ratusan meter hingga kilometer) dan mampu menembus tembok/beton *septic tank*. Oleh karena itu, Node WC di lapangan tidak perlu saling menitip pesan (*hopping/mesh*). Semua Node langsung memancarkan data (*broadcast*) lurus ke satu titik pusat (Menara Posko).
*   **Kelebihan:** Menghemat baterai Node (karena tidak perlu terus menyala untuk mem-*forward* pesan orang lain) dan membebaskan mikrokontroler dari algoritma *routing* yang rawan *error*.

---

## 2. Mental Model & Aktor Jaringan (The 3 Actors)

Sistem ini dibangun oleh 3 entitas (aktor) utama yang bekerja secara terpisah namun terintegrasi:

### Aktor 1: Sang Wartawan Lapangan (ESP32 Node WC)
*   **Perangkat:** ESP32 + Sensor Gas + Sensor Ultrasonik + Antena LoRa RA-02.
*   **Koneksi:** **Hanya Radio LoRa** (Wi-Fi ESP32 dimatikan total untuk menghemat baterai panel surya).
*   **Tugas:** Membaca sensor, meracik string JSON (cth: `{"node_id": "WC_01", "gas": 150}`), dan memancarkannya ke udara lewat frekuensi 433MHz.

### Aktor 2: Sang Jembatan Posko (ESP32 Gateway)
*   **Perangkat:** ESP32 + Antena LoRa RA-02. (Diletakkan di dalam Posko, dicolok ke sumber listrik PLN/Genset stabil).
*   **Koneksi:** Memiliki 2 jalur. Mendengarkan LoRa 433MHz di udara, DAN terhubung ke Wi-Fi Router TP-Link CPE220.
*   **Tugas:** Begitu antena LoRa menangkap paket dari Aktor 1, ESP32 Gateway ini langsung menggunakan *library PubSubClient* untuk melemparkan paket tersebut ke protokol TCP/IP (MQTT) via Wi-Fi.

### Aktor 3: Sang Kantor Pos & Redaksi (Mosquitto & Server Go)
*   **Perangkat:** Laptop/PC Server milik Daffa yang terhubung ke Wi-Fi CPE220. Memiliki IP Statis (misal: `192.168.0.100`).
*   **Mosquitto (Sang Kantor Pos):** Aplikasi MQTT Broker yang berjalan di *background*. Ia bertugas menjaga **Port TCP 1883** tetap terbuka. Menerima JSON dari Aktor 2 dan langsung memindahkannya dari RAM ke Aktor 3.
*   **Server Go (Sang Redaksi):** Berlangganan (*Subscribe*) ke Mosquitto. Saat data masuk, Server Go menaruhnya secara permanen di Brankas (Database PostgreSQL).

---

## 3. Alur Komunikasi Penuh (End-to-End Pipeline)

Berikut adalah perjalanan satu paket data dari kotoran limbah hingga menjadi grafik di layar:

1. **[Fisik]** Sensor Gas MQ-137 membaca konsentrasi amonia 150ppm.
2. **[C++]** ESP32 Node WC membungkusnya jadi teks JSON.
3. **[Radio]** ESP32 Node WC menembakkan JSON ke udara via LoRa 433MHz.
4. **[Radio]** ESP32 Gateway (di posko) menangkap sinyal radio tersebut.
5. **[Wi-Fi]** ESP32 Gateway meneruskannya ke IP `192.168.0.100` pada Port `1883` (Protokol MQTT).
6. **[RAM]** Aplikasi *Mosquitto* di Laptop Daffa menangkap paket itu di dalam RAM, lalu mem-*forward*-nya ke Server Go.
7. **[Storage]** Server Go menyimpannya ke *Hardisk/SSD* (PostgreSQL).
8. **[REST API]** Layar Monitor *Dashboard* menembak REST API (atau WebSocket) ke Server Go untuk meminta data terbaru.
9. **[Visualisasi]** Grafik Amonia di layar bergerak naik ke angka 150.

---

## 4. Alokasi Sumber Daya (*Resource Allocation*)

Untuk menepis kesalahpahaman, protokol MQTT adalah "kurir" tanpa bentuk fisik. Sumber daya komputasi yang bekerja adalah:
*   **SRAM ESP32 (Internal):** Hanya memakan $\approx 1\text{ KB}$ RAM untuk menampung *buffer string* JSON sebelum ditransmisikan.
*   **RAM Laptop Server (Mosquitto):** Hanya memakan $\approx 5 - 15\text{ MB}$ RAM untuk melakukan *In-Memory Routing* (Luar biasa ringan dan cepat, tidak menyebabkan laptop lemot).
*   **SSD Laptop Server (PostgreSQL):** Penyimpanan permanen. Data histori tidak akan pernah hilang meskipun sistem dimatikan atau listrik posko padam.
