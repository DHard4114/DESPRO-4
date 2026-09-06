# DOKUMEN 00: ARCHITECTURE DECISION RECORD (ADR)
## Smart-Sanitation eSOS - Sumber Kebenaran Tunggal (Single Source of Truth)

Dokumen ini adalah **Architecture Decision Record (ADR)** utama yang mencatat seluruh keputusan arsitektural krusial yang telah disepakati oleh tim. Dokumen ini bertindak sebagai payung hukum teknis tertinggi. Jika ada konflik antara kode sumber dan dokumen lain, **ADR ini yang menjadi acuan final**.

---

### ADR-01: Migrasi dari HTTP Ingest ke MQTT
*   **Konteks:** Node sensor sebelumnya dirancang menggunakan Wi-Fi & HTTP POST, yang boros daya dan rentan putus.
*   **Keputusan:** Sistem menggunakan arsitektur **MQTT Broker (Mosquitto)** di Posko. ESP32 Gateway menerima sinyal radio LoRa lalu meneruskannya ke Mosquitto via Wi-Fi Intranet. Go Server berlangganan (*subscribe*) ke Mosquitto.
*   **Konsekuensi:** Beban jaringan Wi-Fi turun drastis, latensi menjadi sub-milidetik, tetapi membutuhkan *setup* Mosquitto di mesin server.

### ADR-02: Topologi Jaringan Sensor "Star" (Point-to-Multipoint)
*   **Konteks:** Pemikiran awal menggunakan topologi *Mesh* agar jangkauan luas.
*   **Keputusan:** Batal menggunakan *Mesh* karena terlalu kompleks untuk Capstone Project. Mengadopsi **Star Topology** menggunakan modul **LoRa RA-02 433MHz**. Semua Node WC (Klien) langsung menembak sinyal ke 1 buah ESP32 Gateway (Pusat) di Posko.
*   **Konsekuensi:** Firmware Node WC murni hanya radio LoRa (tanpa Wi-Fi, menghemat baterai), logika jauh lebih sederhana.

### ADR-03: Backend Go Lambda Architecture
*   **Konteks:** Jutaan baris data sensor akan menumpuk dan membuat *dashboard* menjadi lambat jika di-*query* secara langsung.
*   **Keputusan:** Mengadopsi **Lambda Architecture**. Paket MQTT yang masuk dipecah menjadi dua jalur oleh *Goroutines*:
    1.  **Jalur Stream:** Dikirim seketika via *WebSockets* ke layar *Dashboard*.
    2.  **Jalur Batch:** Disimpan ke *Database* untuk diakumulasi menjadi ringkasan harian/jam.

### ADR-04: Database PostgreSQL Terintegrasi TimescaleDB
*   **Konteks:** Database relasional standar akan kelebihan beban (I/O lambat) jika menerima data IoT setiap 2 detik 24/7.
*   **Keputusan:** Menggunakan **PostgreSQL** yang dipasang ekstensi **TimescaleDB**.
*   **Konsekuensi:** Otomatisasi partisi data (*Hypertables*), kompresi 90%, dan rekapitulasi data historis otomatis (*Continuous Aggregates*).

### ADR-05: Kunci Utama (Primary Key) menggunakan UUIDv7
*   **Konteks:** `Auto-Increment ID` berbahaya jika ada 2 posko berbeda yang nanti disinkronkan ke server BNPB pusat (terjadi tabrakan ID / *ID Collision*).
*   **Keputusan:** Menggunakan **UUIDv7 (Time-Ordered)** sebagai *Primary Key* di semua tabel untuk menjamin keunikan global sekaligus menjaga urutan waktu *insert* agar *query* tetap sangat cepat.

### ADR-06: Antarmuka API berbasis OpenAPI 3.0 (Swagger)
*   **Konteks:** Pengujian manual via Postman melelahkan dan rentan miskomunikasi antara Backend dan Frontend/QA.
*   **Keputusan:** Menulis spesifikasi API berstandar **OpenAPI 3.0**. *Server Go* akan menghasilkan halaman *Swagger UI* yang bisa diklik langsung oleh QA (Raka) untuk *testing*.

### ADR-07: Standardisasi Firmware ESP32 dengan FreeRTOS
*   **Konteks:** Penggunaan `delay()` di C++ konvensional membuat sensor macet saat mengirim data radio.
*   **Keputusan:** Seluruh kode ESP32 Node dan ESP32 Gateway WAJIB menggunakan **FreeRTOS**. Sensor, radio LoRa, dan aktuator berjalan di *Task* yang terpisah (*Multi-Threading*) tanpa saling memblokir.
