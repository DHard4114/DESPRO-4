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

## 5. Serialisasi Payload (JSON Data)
*Standar format pengemasan data sebelum dikirim via LoRa.*
*   **Library Referensi:** [ArduinoJson by Benoit Blanchon](https://arduinojson.org/)
*   **Implementasi RTOS:**
    Diimplementasikan pada *Main Loop* atau sebelum *Task LoRa*. Library ini memungkinkan Amel mengonversi nilai sensor (integer/float) menjadi string berformat `{"gas_nh3": 120, "water_lvl": 80}` yang aman diproses oleh *Go Server* milik Daffa di posko pusat.

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
| TaskLoRaTx | Node WC | Core 1 | 3 | 4096 | Mengirim paket biner ke udara. |
| TaskSensors | Node WC | Core 0 | 1 | 2048 | Membaca pin analog & digital, rata-rata, lalu *Queue*. |
| TaskMqttTx | Gateway | Core 0 | 2 | 4096 | *Dequeue* telemetri, *serialize* ke JSON, *Publish*. |
| TaskActuator | Node WC | Core 0 | 2 | 2048 | Memutar motor servo MG996R via PWM. |


---

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
