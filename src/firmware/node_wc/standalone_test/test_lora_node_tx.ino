/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI MANDIRI PENGIRIM (TRANSMITTER / NODE): ESP32 DEVKIT V1 + Ai-Thinker Ra-02
 * ======================================================================================
 *
 * 1. IDENTIFIKASI BOARD & HARDWARE:
 *    - Target Board   : DOIT ESP32 DevKit V1 (30-pin / ESP32-WROOM-32)
 *    - Modul LoRa     : Ai-Thinker Ra-02 (Semtech SX1278)
 *    - Interface      : Hardware SPI Bus (GPIO Matrix) + Dedicated Control Lines
 *
 * 2. TABEL PENGKABELAN FISIK (WIRING HARNESS SESUAI JUMPER AKTIF):
 *    +-------------------+--------------------+------------------+-----------------------+
 *    | Pin Ra-02 (SX1278)| Pin ESP32 DevKit V1| Warna Kabel Fisik| Fungsi & Catatan      |
 *    +-------------------+--------------------+------------------+-----------------------+
 *    | 3.3V (Kiri Pin 3) | 3V3 (Kanan Pin 1)  | Putih            | Catu Daya 3.3V Stabil |
 *    | GND  (Kiri Pin 2) | GND (Kanan Pin 2)  | Hitam            | Ground Bersama        |
 *    | RST  (Kiri Pin 4) | D15 (Kanan Pin 3)  | Biru             | Reset Hardware SX1278 |
 *    | DIO0 (Kiri Pin 5) | D2  (Kanan Pin 4)  | Ungu             | Interrupt TX/RX Done  |
 *    | NSS  (Kanan Pin 2)| D5  (Kanan Pin 8)  | Kuning           | SPI Chip Select (CS)  |
 *    | MOSI (Kanan Pin 3)| D18 (Kanan Pin 9)  | Oranye           | SPI Master Out Slave In|
 *    | MISO (Kanan Pin 4)| D19 (Kanan Pin 10) | Merah            | SPI Master In Slave Out|
 *    | SCK  (Kanan Pin 5)| D21 (Kanan Pin 11) | Cokelat          | SPI Clock Bus         |
 *    +-------------------+--------------------+------------------+-----------------------+
 *
 * 3. KEPATUHAN REGULASI SPEKTRUM RADIO INDONESIA:
 *    - Regulasi Acuan : Peraturan Menteri Komdigi (Permenkomdigi) No. 2 Tahun 2025.
 *    - Alokasi Pita   : 433,050 MHz – 434,790 MHz (Bandwidth kanal maksimum: 125 kHz).
 *    - Frekuensi Uji  : 433.175 MHz (Pusat kanal rentang nominal).
 *    - Batas Daya     : Maks. 12,15 dBm EIRP (setara 10 dBm ERP / 10 mW).
 *    - Daya Pancar TX : +10 dBm (Conducted).
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// ===================================================================
// 1. DEFINISI PIN PERANGKAT KERAS (SESUAI WIRING JUMPER FISIK AKTIF)
// ===================================================================
#define PIN_LORA_SCK      21    // Cokelat (D21 ESP32)
#define PIN_LORA_MISO     19    // Merah   (D19 ESP32)
#define PIN_LORA_MOSI     18    // Oranye  (D18 ESP32)
#define PIN_LORA_NSS      5     // Kuning  (D5  ESP32)
#define PIN_LORA_DIO0     2     // Ungu    (D2  ESP32)
#define PIN_LORA_RESET    15    // Biru    (D15 ESP32)

// ===================================================================
// 2. PARAMETER MODULASI RF (PATUH REGULASI KOMINFO INDONESIA)
// ===================================================================
#define LORA_FREQ         433.175 // Frekuensi tengah legal (Alokasi: 433,050 - 434,790 MHz)
#define LORA_BW           125.0   // Bandwidth kanal (kHz)
#define LORA_SF           9       // Spreading Factor (SF9: uji ketahanan hambatan bertahap)
#define LORA_CR           7       // Coding Rate 4/7 (CR = 3)
#define LORA_SYNC_WORD    0x12    // Sync Word Jaringan Privat eSOS (SX127X)
#define LORA_TX_POWER     10      // Conducted Power (dBm) - Target EIRP <= 12,15 dBm

// ===================================================================
// 3. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK
// ===================================================================
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Versi skema biner (Selalu 1)
    char node_code[8];      // Offset  1 | 8 bytes : Kode identitas node ("WC_01\0\0\0")
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket monotonik (Little-Endian)
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu operasional sejak boot (Detik, bukan Unix epoch)
    float water_level_cm;   // Offset 17 | 4 bytes : Nilai simulasi sensor air tangki (IEEE-754 Little-Endian)
    float ammonia_ppm;      // Offset 21 | 4 bytes : Nilai simulasi gas amonia (IEEE-754 Little-Endian)
    float h2s_ppm;          // Offset 25 | 4 bytes : Nilai simulasi gas H2S (IEEE-754 Little-Endian)
    float battery_voltage;  // Offset 29 | 4 bytes : Nilai simulasi tegangan baterai 18650 (IEEE-754 Little-Endian)
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : Flag status darurat (0 = Normal, 1 = Darurat)
};

static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// ===================================================================
// 4. INSTANSIASI OBJEK HARDWARE RADIOLIB
// ===================================================================
// Menggunakan SPI 1 MHz untuk kestabilan sinyal tinggi pada kabel jumper breadboard
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0));

// Handle Antrean FreeRTOS
QueueHandle_t xLoRaQueue = NULL;
uint32_t global_packet_counter = 0;

// ===================================================================
// TASK 1: PRODUCER SIMULASI TELEMETRI (Berjalan di Core 0)
// ===================================================================
void vTaskGenerateTelemetry(void *pvParameters) {
    for (;;) {
        TelemetryPayload packet;
        memset(&packet, 0, sizeof(TelemetryPayload));

        packet.schema_version = 1;
        strncpy(packet.node_code, "WC_01", sizeof(packet.node_code) - 1);
        packet.sequence_no = ++global_packet_counter;
        packet.uptime_seconds = (uint32_t)(millis() / 1000);

        packet.water_level_cm = 42.5f;   // Simulasi level tangki
        packet.ammonia_ppm = 8.4f;       // Simulasi amonia
        packet.h2s_ppm = 1.2f;           // Simulasi H2S
        packet.battery_voltage = 3.82f;  // Simulasi Li-ion 18650
        packet.sos_triggered = 0;

        UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[CORE 0 - PRODUCER] Paket #%u dibuat (Ukuran: %u bytes, Uptime: %u s, Stack Free: %u words)\n", 
                      packet.sequence_no, (unsigned int)sizeof(TelemetryPayload), packet.uptime_seconds, (unsigned int)stackRemaining);

        // Kirim ke antrean untuk dieksekusi oleh LoRa Task di Core 1
        if (xQueueSend(xLoRaQueue, &packet, pdMS_TO_TICKS(1000)) != pdTRUE) {
            Serial.printf("[CORE 0 - PERINGATAN] Antrean penuh! Paket #%u gagal dimasukkan.\n", packet.sequence_no);
        }

        // Interval pengujian: 5000 ms
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ===================================================================
// TASK 2: TRANSMISI RADIO LORA Ra-02 (Berjalan di Core 1)
// ===================================================================
void vTaskLoRaTransmitter(void *pvParameters) {
    TelemetryPayload txData;

    for (;;) {
        // Blokir (Sleep) tanpa membebani CPU sampai ada data masuk di antrean
        if (xQueueReceive(xLoRaQueue, &txData, portMAX_DELAY) == pdTRUE) {
            
            UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
            
            // Hitung estimasi teoritis Time-on-Air (ToA)
            float estimatedToA = radio.getTimeOnAir(sizeof(TelemetryPayload)) / 1000.0f; // ms

            Serial.println("-------------------------------------------------------------------");
            Serial.printf("[CORE 1 - LORA TX] Mengambil Paket #%u dari Queue (Stack Free: %u words)\n", 
                          txData.sequence_no, (unsigned int)stackRemaining);
            Serial.printf("  Payload Konten: Node=%s | Uptime=%u s | Air=%.1f cm | NH3=%.1f | H2S=%.1f | Batt=%.2f V\n",
                          txData.node_code, txData.uptime_seconds, txData.water_level_cm, 
                          txData.ammonia_ppm, txData.h2s_ppm, txData.battery_voltage);
            Serial.printf("  RF Config     : Frek=%.3f MHz | SF=%d | BW=%.1f kHz | Daya=%d dBm\n",
                          LORA_FREQ, LORA_SF, LORA_BW, LORA_TX_POWER);
            Serial.printf("  Estimasi ToA  : %.2f ms (Formula Semtech SX1278)\n", estimatedToA);

            unsigned long startExecTime = millis();
            
            // Transmisi sinkronus biner via chip SX1278
            int txState = radio.transmit((uint8_t*)&txData, sizeof(TelemetryPayload));
            
            unsigned long measuredDuration = millis() - startExecTime;

            if (txState == RADIOLIB_ERR_NONE) {
                Serial.printf(">>> [TX SUKSES LOKAL] Sinyal TX_DONE diterima dari pin DIO0 (GPIO %d)!\n", PIN_LORA_DIO0);
                Serial.printf(">>> Durasi Pemanggilan Fungsi (SPI + ToA): %lu ms\n", measuredDuration);
                Serial.println(">>> CATATAN: Status ini HANYA menandakan transmisi lokal selesai dipancarkan.");
                Serial.println("    Keberhasilan penerimaan paket di sisi posko memerlukan verifikasi unit RX.");
            } else {
                Serial.printf(">>> [TX GAGAL] Kode Kesalahan RadioLib: %d\n", txState);
                Serial.println("    Periksa kestabilan catu daya 3.3V dan kontinuitas kabel pin DIO0.");
            }
            Serial.println("-------------------------------------------------------------------\n");
        }
    }
}

// ===================================================================
// SETUP (INISIALISASI SISTEM & DIAGNOSTIK)
// ===================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Waktu stabilisasi Serial UART

    Serial.println("\n\n===================================================================");
    Serial.println("   SMART-SANITATION eSOS - UJI LORA NODE TRANSMITTER (ESP32 DevKit V1) ");
    Serial.println("===================================================================");
    Serial.println("Parameter Teknis & Regulasi:");
    Serial.printf("  - Target Board        : DOIT ESP32 DevKit V1 (30-pin)\n");
    Serial.printf("  - Modul Radio         : Semtech SX1278 (Ai-Thinker Ra-02)\n");
    Serial.printf("  - Frekuensi Operasi   : %.3f MHz (Legal: 433,050 - 434,790 MHz)\n", LORA_FREQ);
    Serial.printf("  - Bandwidth Kanal     : %.1f kHz\n", LORA_BW);
    Serial.printf("  - Spreading Factor    : SF%d\n", LORA_SF);
    Serial.printf("  - Coding Rate         : 4/%d\n", LORA_CR);
    Serial.printf("  - Sync Word           : 0x%02X (Jaringan Privat)\n", LORA_SYNC_WORD);
    Serial.printf("  - Daya Pancar Chip    : +%d dBm (Target EIRP <= 12,15 dBm Kominfo)\n", LORA_TX_POWER);
    Serial.printf("  - Ukuran Struct Biner : %u Bytes (static_assert terverifikasi)\n", (unsigned int)sizeof(TelemetryPayload));
    Serial.printf("  - Pemetaan Pin Jumper : SCK:%d (Cokelat), MOSI:%d (Oranye), MISO:%d (Merah), NSS:%d (Kuning), RST:%d (Biru), DIO0:%d (Ungu)\n",
                  PIN_LORA_SCK, PIN_LORA_MOSI, PIN_LORA_MISO, PIN_LORA_NSS, PIN_LORA_RESET, PIN_LORA_DIO0);
    Serial.println("===================================================================");

    // 0. Pre-inisialisasi NSS ke HIGH agar SX1278 tidak merespons noise boot ESP32
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    // 1. Hard Reset Hardware SX1278 (Durasi 20ms LOW, lalu 100ms settling time)
    Serial.print("1. Melakukan Hardware Reset SX1278 (RST:15)... ");
    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100); // Memberikan waktu agar kristal osilator 32 MHz dan regulator stabil
    Serial.println("[OK]");

    // 2. Inisialisasi Hardware SPI Bus (SS=-1 agar tidak konflik dengan kontrol manual CS di RadioLib)
    Serial.printf("2. Inisialisasi Hardware SPI Bus (SCK:%d, MISO:%d, MOSI:%d, SS:Manual)... ",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);
    Serial.println("[OK]");

    // 3. Inisialisasi Radio SX1278 via RadioLib dengan retry loop
    Serial.print("3. Menghubungi Register Chip Semtech SX1278... ");
    int initState = RADIOLIB_ERR_UNKNOWN;
    for (int attempt = 1; attempt <= 3; attempt++) {
        initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
        if (initState == RADIOLIB_ERR_NONE) break;
        delay(50);
    }

    if (initState == RADIOLIB_ERR_NONE) {
        Serial.println("[OK]");
        Serial.println("   -> Chip SX1278 merespons normal pada bus SPI.");
        Serial.println("   -> PERINGATAN FISIK: Pastikan antena 433 MHz terpasang kencang!");
        Serial.println("      Respon SPI TIDAK mendeteksi ada/tidaknya antena fisik.");
        
        // Aktifkan hardware CRC secara eksplisit agar cocok dengan sisi penerima
        int crcState = radio.setCRC(true);
        if (crcState != RADIOLIB_ERR_NONE) {
            Serial.printf("   -> [PERINGATAN] Gagal mengaktifkan CRC hardware: %d\n", crcState);
        } else {
            Serial.println("   -> Hardware CRC: [AKTIF]");
        }
    } else {
        Serial.println("[GAGAL]");
        Serial.printf("   -> Kode Galat RadioLib: %d\n", initState);
        Serial.println("   -> Langkah Diagnostik:");
        Serial.printf("      1. Periksa kabel SPI (MOSI:%d, MISO:%d, SCK:%d, NSS:%d, RST:%d, DIO0:%d).\n",
                      PIN_LORA_MOSI, PIN_LORA_MISO, PIN_LORA_SCK, PIN_LORA_NSS, PIN_LORA_RESET, PIN_LORA_DIO0);
        Serial.println("      2. Pastikan tegangan VCC adalah 3.3V stabil (Bukan 5V).");
        Serial.println("      3. Periksa pin DIO0 (GPIO 2) tidak tertahan tegangan tinggi saat boot.");
        Serial.println("Sistem dihentikan.");
        while (true) delay(1000);
    }

    // 4. Alokasi Antrean FreeRTOS
    Serial.print("4. Mengalokasikan FreeRTOS Queue (Kapasitas: 5 Paket)... ");
    xLoRaQueue = xQueueCreate(5, sizeof(TelemetryPayload));
    if (xLoRaQueue == NULL) {
        Serial.println("[GAGAL]");
        Serial.println("   -> Alokasi memori heap untuk Queue gagal! Sistem berhenti.");
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // 5. Pembagian Multitasking FreeRTOS ke Dual-Core
    Serial.println("5. Memasang Task FreeRTOS ke Dual Core ESP32:");
    
    BaseType_t taskSensorsStatus = xTaskCreatePinnedToCore(
        vTaskGenerateTelemetry, 
        "TaskSensors", 
        3072,   // Stack 3072 words
        NULL, 
        1,      // Prioritas 1
        NULL, 
        0       // Core 0 (PRO_CPU)
    );

    BaseType_t taskLoRaStatus = xTaskCreatePinnedToCore(
        vTaskLoRaTransmitter, 
        "TaskLoRaTx", 
        4096,   // Stack 4096 words
        NULL, 
        2,      // Prioritas 2
        NULL, 
        1       // Core 1 (APP_CPU)
    );

    if (taskSensorsStatus != pdPASS || taskLoRaStatus != pdPASS) {
        Serial.println("[GAGAL]");
        Serial.println("   -> Gagal membuat salah satu Task FreeRTOS! Periksa sisa heap.");
        while (true) delay(1000);
    }
    Serial.println("   -> TaskSensors (Core 0, Prio 1) : [BERHASIL DIBUAT]");
    Serial.println("   -> TaskLoRaTx  (Core 1, Prio 2) : [BERHASIL DIBUAT]");
    Serial.println("      (Pesan ini menandakan alokasi task berhasil, bukan kepastian transmisi berulang)");
    Serial.println("===================================================================\n");
}

// ===================================================================
// LOOP FUNCTION (Perilaku FreeRTOS pada Arduino-ESP32)
// ===================================================================
void loop() {
    vTaskDelete(NULL);
}
