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
