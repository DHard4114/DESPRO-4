# REFERENSI IMPLEMENTASI FIRMWARE & SENSOR (ESP32 FreeRTOS)

Dokumen ini berisi daftar pustaka (library), referensi *datasheet*, dan panduan implementasi *coding* berbasis **FreeRTOS** untuk setiap sensor dan aktuator pada proyek *Smart-Sanitation eSOS*. 

Menggunakan arsitektur RTOS (*Real-Time Operating System*) berarti kita tidak menggunakan `delay()` biasa yang memblokir proses, melainkan menggunakan `vTaskDelay()` dan **Task/Queue** yang berjalan secara konkuren (bersamaan) di *Dual-Core* ESP32.

---

## 0. Core Kernel: ESP32 FreeRTOS (Arsitektur Utama)
*   **Referensi:** [FreeRTOS Kernel Book (Ch. 1)](https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch01.md#111-about-the-freertos-kernel)
*   **Implementasi:** Sistem akan membagi pekerjaan menjadi beberapa *Task* mandiri (`xTaskCreatePinnedToCore`). Misalnya, pembacaan sensor gas di *Core 0*, dan transmisi LoRa di *Core 1*. Perpindahan data antar sensor menggunakan *FreeRTOS Queues*.

---

## 1. Sensor Ultrasonik Waterproof (JSN-SR04T)
*Sensor untuk membaca level ketinggian limbah air.*
*   **Library Referensi:** [NewPing by teckel12](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home) (Lebih presisi dari pulseIn standar)
*   **Referensi Datasheet:** [JSN-SR04T Specifications](https://www.makerguides.com/jsn-sr04t-arduino-tutorial/)
*   **Implementasi RTOS:** 
    Dibuat dalam `vTaskUltrasonic`. Sensor ultrasonik membutuhkan waktu *ping* sekitar 50ms. Gunakan `vTaskDelay(pdMS_TO_TICKS(50))` agar saat menunggu sinyal suara memantul, ESP32 bisa memproses tugas lain (tidak nge-hang).
*   **Mitigasi Perangkat Keras:** Sensor JSN-SR04T memiliki blind zone bawaan 20-25 cm. Dalam logika `vTaskSensors`, wajib ditambahkan *filtering*: Jika hasil bacaan ping < 25 cm atau bernilai 0 (timeout dekat), maka firmware WAJIB memaksa nilai menjadi 25 cm (Tangki Kritis/Penuh). Ini untuk mencegah data sampah (garbage value) memicu false alarm di server.

---

## 2. Sensor Gas Amonia (MQ-137) & Hidrogen Sulfida (MQ-136)
*Sensor analog mendeteksi konsentrasi gas berbahaya.*
*   **Library Referensi:** [MQUnifiedsensor by Miguel A. Vallejo](https://github.com/miguel5612/MQSensorsLib)
    *(Library ini sangat penting karena sudah memiliki rumus konversi ADC ke PPM berdasarkan grafik $R_s/R_0$ dari datasheet pabrik Hanwei).*
*   **Referensi Datasheet:**
    *   [Datasheet MQ-137 (Amonia)](https://www.winsen-sensor.com/d/files/semiconductor/mq137.pdf)
    *   [Datasheet MQ-136 (H2S)](https://www.winsen-sensor.com/d/files/semiconductor/mq136.pdf)
*   **Implementasi RTOS:**
    Dibuat dalam `vTaskGasSensors`. Karena nilai analog gas sering fluktuatif, *task* ini harus melakukan *Moving Average* (mengambil 10 pembacaan lalu dirata-rata). Data PPM akhirnya dikirim ke *FreeRTOS Queue* menuju modul LoRa.

---

## 3. Modul Transceiver Telemetri (LoRa RA-02 433MHz / SX1278)
*Sistem komunikasi jarak jauh bebas kuota internet.*
*   **Library Referensi:** [LoRa by Sandeep Mistry](https://github.com/sandeepmistry/arduino-LoRa) (Standard & Mudah) atau [RadioLib by jgromes](https://github.com/jgromes/RadioLib) (Lebih *Advanced* untuk tuning *Spreading Factor*).
*   **Referensi Datasheet:** [Semtech SX1278 Datasheet](https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001Rbr/6EfVZUorrpoKFfvaF_Fkpgp5kzjiNyiAbqcpW9KES%3E)
*   **Implementasi RTOS:**
    Dibuat dalam `vTaskLoRaTx`. *Task* ini dalam mode *Blocking* (menunggu) pada suatu *Queue*. Begitu data JSON sensor masuk ke *Queue*, *Task* LoRa langsung terbangun (*wake up*) dan memancarkan paket via jalur komunikasi **SPI**, lalu kembali tidur (*sleep*).

---

## 4. Motor Servo Aktuator (MG996R)
*Membuka/menutup katup ventilasi exhaust atau penjatuh cairan pengurai.*
*   **Library Referensi:** [ESP32Servo by Kevin Harrington](https://github.com/madhephaestus/ESP32Servo)
    *(PENTING: Jangan gunakan `Servo.h` standar Arduino karena akan bentrok dengan timer perangkat keras internal ESP32. Gunakan `ESP32Servo`).*
*   **Referensi Datasheet:** [MG996R TowerPro Specifications](https://www.electronicoscaldas.com/datasheet/MG996R_Tower-Pro.pdf)
*   **Implementasi RTOS:**
    Dibuat dalam `vTaskActuator`. Aktuator dikontrol menggunakan sinyal PWM perangkat keras. *Task* ini hanya merespons jika ada pesan "PERINTAH BUKA" atau jika nilai Amonia di *Queue* melebihi batas bahaya (> 200 ppm).

---

## 5. Serialisasi Payload (Binary vs JSON)

*Standar format pengemasan data harus dibedakan antara transmisi Radio dan transmisi Wi-Fi.*

*   **Node WC (LoRa Tx):** TIDAK MENGGUNAKAN JSON. Menggunakan struktur data C standar (struct Payload { ... }) yang dikirim mentah (*raw memory copy*) untuk efisiensi SRAM dan *airtime* di udara. **Seluruh struktur data (C-Struct) payload biner yang ditransmisikan via LoRa WAJIB menggunakan atribut __attribute__((packed)) untuk mencegah kompilator C++ menyisipkan byte kosong (Memory Alignment Padding). Ukuran struct maksimal dibatasi 50 bytes. Gateway akan membaca memory block ini secara langsung (raw memory cast).**

`cpp
// KONTRAK PAYLOAD BINER MUTLAK (Max 50 Bytes)
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Selalu 1
    char node_code[8];      // Contoh: "WC_01" (Null terminated)
    uint32_t sequence_no;   // Counter pesan
    float water_level_cm;
    float ammonia_ppm;
    float h2s_ppm;
    float battery_voltage;
    uint8_t sos_triggered;  // 1 = True, 0 = False
};
`

*   **Gateway (Wi-Fi Tx):** Menggunakan library **ArduinoJson** untuk men-*deserialize* struct biner tadi dan merakitnya menjadi String JSON Envelope utuh sebelum dikirim ke Server Mosquitto.

---

## 6. Topologi Antrean Data (FreeRTOS IPC & Queue Topology)

Sebagai fondasi *thread-safety* antar-*task*, dilarang keras melakukan pertukaran data antar *Goroutine/Task* menggunakan variabel global publik (Global Variables) tanpa pelindung. Sistem wajib menggunakan **FreeRTOS Queue (xQueue)** dan **Interrupt Safe API (...FromISR)**.

### 6.1 Topologi Queue pada ESP32 Node WC (Transmitter)
*   QueueHandle_t xQueueSensorData;
    *   **Produsen:** TaskSensors (Ultrasonik & Gas).
    *   **Konsumen:** TaskLoRaTx.
    *   **Mekanisme:** Sensor membaca data dan mengirim struct TelemetryPayload ke Queue ini. LoRa Tx menunggu secara *blocking* (portMAX_DELAY), sehingga CPU tidak sibuk (*idle*) sampai ada data masuk.
*   **Penanganan Interrupt (Tombol SOS):**
    *   Tombol SOS memicu rutin ISR (Interrupt Service Routine).
    *   ISR memanggil xQueueSendFromISR(xQueueSensorData, &emergencyPayload, &xHigherPriorityTaskWoken).
    *   Ini langsung membangunkan TaskLoRaTx tanpa harus menunggu siklus bacaan sensor normal selesai.

### 6.2 Topologi Queue pada ESP32 Gateway (Jembatan)
*   QueueHandle_t xQueueTelemetry;
    *   **Produsen:** TaskLoRaRx (Menangkap paket LoRa dari udara).
    *   **Konsumen:** TaskMqttTx (Meneruskan ke Wi-Fi Mosquitto).
    *   **Integrasi ADR-03:** Jika Mosquitto mati/Wi-Fi putus, TaskMqttTx akan gagal mengirim. Data di-pop dari Queue dan dilempar ke LittleFS (*Store-and-Forward*).
*   QueueHandle_t xQueueCommand;
    *   **Produsen:** TaskMqttRx (Menerima perintah buka/tutup katup dari Dashboard).
    *   **Konsumen:** TaskLoRaTx (Gateway menembakkan perintah balik ke Node).


### 6.3 Pemetaan Task FreeRTOS (Node & Gateway)

Spesifikasi mutlak untuk parameter *Task* (dilarang diubah saat implementasi):

| Nama Task | Target Board | Core (Affinity) | Priority | Stack Size (Bytes) | Deskripsi |
| :--- | :--- | :---: | :---: | :---: | :--- |
| TaskLoRaRx | Gateway | Core 1 | 3 | 4096 | Menangkap paket biner LoRa secara kontinu. |
| TaskLoRaTx | Node WC | Core 1 | 3 | 4096 | Mengirim paket biner ke udara. SETELAH transmisi TX selesai, task ini WAJIB membuka RX Window (mendengarkan) selama 2000ms. Jika dalam 2000ms menerima paket Downlink (Perintah Aktuator), teruskan ke xQueueCommand. Jika timeout, kembali tidur. Mekanisme ini mirip LoRaWAN Class A. |
| TaskSensors | Node WC | Core 0 | 1 | 2048 | Membaca pin analog & digital, rata-rata, lalu *Queue*. |
| TaskMqttTx | Gateway | Core 0 | 2 | 4096 | *Dequeue* telemetri, *serialize* ke JSON, *Publish*. |
| TaskActuator | Node WC | Core 0 | 2 | 2048 | Memutar motor servo MG996R via PWM. |


---


### 6.4 Hardware Interrupts (ISR) & Preemptive Scheduler

1. **Preemptive Scheduler:** FreeRTOS berjalan dalam mode Preemptive. Task berprioritas tinggi (LoRa) akan otomatis menyela (preempt) task berprioritas rendah (Sensor) jika ada data yang masuk ke antrean.
2. **Tombol SOS (EXTI ISR):** Tombol fisik SOS dilarang dibaca via *polling* digitalRead. Wajib dihubungkan ke pin Hardware Interrupt. Saat ditekan, rutin ISR akan memanggil xQueueSendFromISR yang secara paksa dan instan membangunkan TaskLoRaTx (Bypass antrean normal).
3. **LoRa DIO0 (Non-Blocking ISR):** Untuk memastikan radio non-blocking, pin DIO0 pada SX1278 (LoRa) wajib memicu Interrupt saat paket biner selesai dikirim (TX Done) atau diterima (RX Done), memberikan *semaphore* ke task terkait agar CPU bisa tidur/yield selama transmisi berlangsung.

## 7. Manajemen Build (PlatformIO)

Firmware untuk kedua peran fisik (*Node* dan *Gateway*) disatukan dalam satu repositori yang sama untuk kemudahan *sharing* struct biner (Payload), namun **wajib dipisah secara ketat** pada konfigurasi *build* platformio.ini.

`ini
[env:node_wc]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
build_flags = -D IS_NODE_WC
lib_deps =
    jgromes/RadioLib
    # DILARANG KERAS memasukkan library Wi-Fi atau MQTT di env ini!

[env:gateway_router]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
build_flags = -D IS_GATEWAY
lib_deps =
    jgromes/RadioLib
    knolleary/PubSubClient
`


---

## 8. Konfigurasi Global (config.h)

Dilarang melakukan hardcode identitas node (seperti "WC_01") menyebar di dalam file .cpp. Seluruh identitas node, pin GPIO, dan parameter LoRa (Frekuensi 433E6, Spreading Factor, dll) WAJIB dipusatkan di dalam file src/firmware/include/config.h.
