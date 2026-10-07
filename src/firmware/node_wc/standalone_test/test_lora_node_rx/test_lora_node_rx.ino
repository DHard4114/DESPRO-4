/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI MANDIRI PENERIMA (RECEIVER / NODE RX): ESP32 DEVKIT V1 + Ai-Thinker Ra-02
 * Target Node   : Node WC (Bilik Sanitasi)
 * ======================================================================================
 *
 * 1. IDENTIFIKASI BOARD & HARDWARE:
 *    - Target Board   : DOIT ESP32 DevKit V1 (30-pin, ESP32-WROOM-32)
 *    - Modul Radio    : Ai-Thinker Ra-02 (Semtech SX1278)
 *    - Interface      : Hardware SPI Bus + Dedicated Control Lines
 *    - Catu Daya      : 3.3V Stabil (DILARANG 5V/VIN)
 *
 * 2. TABEL PENGKABELAN JUMPER FISIK AKTIF (Identik dengan Node WC):
 *    +-------------------+--------------------+------------------+-----------------------+
 *    | Pin Ra-02 (SX1278)| Pin ESP32 DevKit V1| Warna Kabel Fisik| Fungsi & Catatan      |
 *    +-------------------+--------------------+------------------+-----------------------+
 *    | 3.3V (Kiri Pin 3) | 3V3 (Kanan Pin 1)  | Putih            | Catu Daya 3.3V Stabil |
 *    | GND  (Kiri Pin 2) | GND (Kanan Pin 2)  | Hitam            | Ground Bersama        |
 *    | RST  (Kiri Pin 4) | D15 (Kanan Pin 3)  | Biru             | Reset Hardware SX1278 |
 *    | DIO0 (Kiri Pin 5) | D2  (Kanan Pin 4)  | Ungu             | Interrupt RX_DONE     |
 *    | NSS  (Kanan Pin 2)| D5  (Kanan Pin 8)  | Kuning           | SPI Chip Select (CS)  |
 *    | MOSI (Kanan Pin 3)| D18 (Kanan Pin 9)  | Oranye           | SPI Master Out Slave In|
 *    | MISO (Kanan Pin 4)| D19 (Kanan Pin 10) | Merah            | SPI Master In Slave Out|
 *    | SCK  (Kanan Pin 5)| D21 (Kanan Pin 11) | Cokelat          | SPI Clock Bus         |
 *    +-------------------+--------------------+------------------+-----------------------+
 *
 * 3. KEPATUHAN REGULASI SPEKTRUM RADIO INDONESIA:
 *    - Regulasi Acuan : Peraturan Menteri Komdigi No. 2 Tahun 2025 (Pita LPWAN/SRD).
 *    - Alokasi Pita   : 433,050 MHz – 434,790 MHz (Bandwidth kanal maksimum: 125 kHz).
 *    - Frekuensi Uji  : 433.175 MHz.
 *    - Modulasi       : SF9 | BW 125.0 kHz | CR 4/7 | SyncWord 0x12.
 *
 * 4. ARSITEKTUR FREERTOS:
 *    - Core 1 (APP_CPU) : TaskLoRaRx (Prioritas 3) — Mendengarkan interupsi RX hardware DIO0
 *    - Core 0 (PRO_CPU) : TaskProcessor (Prioritas 1) — Pemilik tunggal Serial & parser paket
 *    - loop() dihapus via vTaskDelete(NULL) untuk efisiensi memori stack.
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// ===================================================================
// 1. DEFINISI PIN PERANGKAT KERAS (JUMPER AKTIF NODE WC)
// ===================================================================
#define PIN_LORA_SCK      21    // Cokelat (D21 ESP32)
#define PIN_LORA_MISO     19    // Merah   (D19 ESP32)
#define PIN_LORA_MOSI     18    // Oranye  (D18 ESP32)
#define PIN_LORA_NSS      5     // Kuning  (D5  ESP32)
#define PIN_LORA_DIO0     2     // Ungu    (D2  ESP32) - Interupsi RX_DONE
#define PIN_LORA_RESET    15    // Biru    (D15 ESP32) - Reset Hardware

// ===================================================================
// 2. PARAMETER MODULASI RF (IDENTIK DENGAN TRANSMITTER STANDALONE)
// ===================================================================
#define LORA_FREQ         433.175
#define LORA_BW           125.0
#define LORA_SF           9
#define LORA_CR           7
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     10

// Identitas Node ini
#define TARGET_NODE_CODE  "WC_01"

// ===================================================================
// 3. KONTRAK STRUKTUR PAYLOAD BINER
// ===================================================================
// Downlink Command dari Posko/Gateway (10 Bytes)
struct __attribute__((packed)) ActuatorCommand {
    char    node_code[8]; // Target Node ("WC_01\0\0\0")
    uint8_t command_id;   // 1 = LOCK_DOOR, 2 = UNLOCK_DOOR, 3 = FLUSH, 4 = PING
    uint8_t parameter;    // Sudut servo (0-180) atau parameter kontrol
};
static_assert(sizeof(ActuatorCommand) == 10, "FATAL: Ukuran ActuatorCommand harus tepat 10 bytes!");

// Telemetry Payload dari Node (34 Bytes)
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte
    char node_code[8];      // Offset  1 | 8 bytes
    uint32_t sequence_no;   // Offset  9 | 4 bytes
    uint32_t uptime_seconds;// Offset 13 | 4 bytes
    float water_level_cm;   // Offset 17 | 4 bytes
    float ammonia_ppm;      // Offset 21 | 4 bytes
    float h2s_ppm;          // Offset 25 | 4 bytes
    float battery_voltage;  // Offset 29 | 4 bytes
    uint8_t sos_triggered;  // Offset 33 | 1 byte
};
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// Wrapper Antrean Paket RX (Maksimum 64 bytes buffer mentah)
struct NodeRxPacket {
    uint8_t raw_data[64];
    size_t length;
    float rssi;
    float snr;
    float freq_error_hz;
    uint32_t rx_timestamp_ms;
};

// ===================================================================
// 4. INSTANSIASI PERANGKAT KERAS & KERNEL FREERTOS
// ===================================================================
// RadioLib SX1278 (SPI 1 MHz untuk kestabilan sinyal tinggi pada kabel jumper)
static SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0));

static QueueHandle_t xQueueRxPackets = NULL;
static TaskHandle_t xTaskLoRaRxHandle = NULL;
static TaskHandle_t xTaskProcessorHandle = NULL;

static volatile uint32_t g_rx_packet_count = 0;

// ===================================================================
// 5. ISR INTERRUPSI DIO0 (HARDWARE RX_DONE)
// ===================================================================
void IRAM_ATTR isr_lora_rx() {
    if (xTaskLoRaRxHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(xTaskLoRaRxHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ===================================================================
// 6. TASK 1: PENERIMA RADIO ASINKRON (CORE 1, PRIORITAS 3)
// ===================================================================
void vTaskLoRaRx(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        // Blokir tanpa membebani CPU hingga ada interupsi hardware DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        size_t packetLen = radio.getPacketLength();
        if (packetLen > 0 && packetLen <= sizeof(((NodeRxPacket*)0)->raw_data)) {
            NodeRxPacket pkt;
            memset(&pkt, 0, sizeof(NodeRxPacket));

            int state = radio.readData(pkt.raw_data, packetLen);
            if (state == RADIOLIB_ERR_NONE) {
                pkt.length = packetLen;
                pkt.rssi = radio.getRSSI();
                pkt.snr = radio.getSNR();
                pkt.freq_error_hz = radio.getFrequencyError();
                pkt.rx_timestamp_ms = millis();
                g_rx_packet_count++;

                if (xQueueRxPackets != NULL) {
                    if (xQueueSend(xQueueRxPackets, &pkt, 0) != pdPASS) {
                        // Antrean penuh: drop frame tertua bila perlu
                        NodeRxPacket dummy;
                        xQueueReceive(xQueueRxPackets, &dummy, 0);
                        xQueueSend(xQueueRxPackets, &pkt, 0);
                    }
                }
            }
        }

        // Aktifkan kembali mode RX Continuous
        radio.startReceive();
    }
}

// ===================================================================
// 7. TASK 2: PEMROSES & LOGGER DATA (CORE 0, PRIORITAS 1) — SOLE WRITER
// ===================================================================
void vTaskProcessor(void *pvParameters) {
    (void)pvParameters;
    NodeRxPacket pkt;

    for (;;) {
        if (xQueueReceive(xQueueRxPackets, &pkt, portMAX_DELAY) == pdTRUE) {
            Serial.println(F("========================================================================"));
            Serial.printf("[NODE RX #%05u] Monotonic Uptime: %lu ms | Ukuran: %u Bytes\n",
                          g_rx_packet_count, pkt.rx_timestamp_ms, (unsigned int)pkt.length);
            Serial.printf("  Metrik RF : RSSI = %6.1f dBm | SNR = %5.1f dB | Freq Error = %7.1f Hz\n",
                          pkt.rssi, pkt.snr, pkt.freq_error_hz);

            // Hex Dump Payload Mentah
            Serial.print(F("  Hex Dump  : "));
            for (size_t i = 0; i < pkt.length; i++) {
                Serial.printf("%02X ", pkt.raw_data[i]);
            }
            Serial.println();

            // Dekoding berdasarkan ukuran paket
            if (pkt.length == sizeof(ActuatorCommand)) {
                ActuatorCommand* cmd = (ActuatorCommand*)pkt.raw_data;
                bool isForThisNode = (strncmp(cmd->node_code, TARGET_NODE_CODE, sizeof(cmd->node_code)) == 0);

                Serial.println(F("  Dekoding  : [KONTRAK DOWNLINK ACTUATOR COMMAND (10 Bytes)]"));
                Serial.printf("    Target Node : %s (%s)\n",
                              cmd->node_code, isForThisNode ? "COCOK / UNTUK NODE INI" : "DIABAIKAN / UNTUK NODE LAIN");
                Serial.printf("    Command ID  : %u ", cmd->command_id);
                switch (cmd->command_id) {
                    case 1: Serial.println(F("[LOCK_DOOR - Kunci Bilik WC]")); break;
                    case 2: Serial.println(F("[UNLOCK_DOOR - Buka Kunci Bilik WC]")); break;
                    case 3: Serial.println(F("[FLUSH - Siram Katup Otomatis]")); break;
                    case 4: Serial.println(F("[PING / ECHO CHECK]")); break;
                    default: Serial.println(F("[CUSTOM / UNKNOWN COMMAND]")); break;
                }
                Serial.printf("    Parameter   : %u\n", cmd->parameter);
            } 
            else if (pkt.length == sizeof(TelemetryPayload)) {
                TelemetryPayload* tel = (TelemetryPayload*)pkt.raw_data;
                Serial.println(F("  Dekoding  : [KONTRAK TELEMETRI UPLINK (34 Bytes)]"));
                Serial.printf("    Node Code   : %s\n", tel->node_code);
                Serial.printf("    Sequence No : #%u\n", tel->sequence_no);
                Serial.printf("    Uptime Node : %u s\n", tel->uptime_seconds);
                Serial.printf("    Level Air   : %.1f cm\n", tel->water_level_cm);
                Serial.printf("    Amonia (NH3): %.2f\n", tel->ammonia_ppm);
                Serial.printf("    H2S         : %.2f\n", tel->h2s_ppm);
                Serial.printf("    Tegangan Bat: %.2f V\n", tel->battery_voltage);
                Serial.printf("    Status SOS  : %u\n", tel->sos_triggered);
            } 
            else {
                Serial.println(F("  Dekoding  : [PAKET RAW / UKURAN TIDAK DIKENAL]"));
            }

            // Pemantauan Sisa Stack FreeRTOS (Satuan BYTES)
            UBaseType_t stackRem = uxTaskGetStackHighWaterMark(NULL);
            Serial.printf("  [RTOS] Sisa Stack TaskProcessor: %u Bytes\n", (unsigned int)stackRem);
            Serial.println(F("========================================================================\n"));
        }
    }
}

// ===================================================================
// 8. SETUP SISTEM UTAMA
// ===================================================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Waktu stabilisasi Serial UART

    Serial.println();
    Serial.println(F("========================================================================"));
    Serial.println(F(" Smart-Sanitation eSOS — Uji Mandiri LoRa Node RX [v2.0]                "));
    Serial.println(F(" Subsystem: Penerima LoRa SX1278 (Ai-Thinker Ra-02) pada Node WC       "));
    Serial.println(F(" Framework: Arduino-ESP32 v2.0.17 / Native Espressif FreeRTOS           "));
    Serial.println(F("========================================================================"));
    Serial.println(F("Parameter RF & Regulasi:"));
    Serial.printf("  - Target Node ID    : %s\n", TARGET_NODE_CODE);
    Serial.printf("  - Frekuensi Tengah  : %.3f MHz (Permenkomdigi No. 2 Tahun 2025)\n", LORA_FREQ);
    Serial.printf("  - Bandwidth Kanal   : %.1f kHz\n", LORA_BW);
    Serial.printf("  - Spreading Factor  : SF%d\n", LORA_SF);
    Serial.printf("  - Coding Rate       : 4/%d\n", LORA_CR);
    Serial.printf("  - Sync Word         : 0x%02X\n", LORA_SYNC_WORD);
    Serial.printf("  - Pin Wiring SX1278 : SCK:%d, MISO:%d, MOSI:%d, NSS:%d, RST:%d, DIO0:%d\n",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS, PIN_LORA_RESET, PIN_LORA_DIO0);
    Serial.println(F("========================================================================"));

    // 1. Alokasi Antrean Paket FreeRTOS
    xQueueRxPackets = xQueueCreate(8, sizeof(NodeRxPacket));
    if (xQueueRxPackets == NULL) {
        Serial.println(F("[FATAL] Gagal membuat antrean FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 2. Pre-inisialisasi NSS ke HIGH agar SX1278 tidak merespons noise boot ESP32
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    // 3. Hardware Reset SX1278 (LOW 20 ms, Settling time 100 ms)
    Serial.print(F("1. Melakukan Hardware Reset SX1278 (RST:15)... "));
    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);
    Serial.println(F("[OK]"));

    // 4. Inisialisasi Bus Hardware SPI
    Serial.printf("2. Inisialisasi Hardware SPI (SCK:%d, MISO:%d, MOSI:%d, NSS:%d)... ",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);
    Serial.println(F("[OK]"));

    // 5. Inisialisasi Register Radio SX1278 dengan Retry Loop
    Serial.print(F("3. Menghubungi Register Chip Semtech SX1278... "));
    int initState = RADIOLIB_ERR_UNKNOWN;
    for (int attempt = 1; attempt <= 3; attempt++) {
        initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
        if (initState == RADIOLIB_ERR_NONE) break;
        delay(50);
    }

    if (initState == RADIOLIB_ERR_NONE) {
        Serial.println(F("[OK]"));
        radio.setCRC(true);
        Serial.println(F("   -> Radio SX1278 berhasil diinisialisasi (433.175 MHz)."));
    } else {
        Serial.printf("[GAGAL] Kode kesalahan: %d\n", initState);
        Serial.println(F("   -> Periksa kabel SPI dan jumper daya 3.3V ke SX1278."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 6. Konfigurasi Interupsi DIO0 untuk Penerimaan Asinkron
    Serial.print(F("4. Mengaktifkan Interupsi Hardware DIO0 (Pin D2)... "));
    radio.setDio0Action(isr_lora_rx, RISING);
    int rxState = radio.startReceive();
    if (rxState == RADIOLIB_ERR_NONE) {
        Serial.println(F("[OK] (Mode RX Continuous Aktif)"));
    } else {
        Serial.printf("[GAGAL] Kode kesalahan startReceive: %d\n", rxState);
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 7. Meluncurkan Task FreeRTOS ke Dual Core
    Serial.println(F("5. Meluncurkan Task FreeRTOS ke Dual Core..."));

    BaseType_t retProcessor = xTaskCreatePinnedToCore(
        vTaskProcessor,
        "TaskProcessor",
        4096,
        NULL,
        1,
        &xTaskProcessorHandle,
        0 // Core 0 (PRO_CPU)
    );

    BaseType_t retRx = xTaskCreatePinnedToCore(
        vTaskLoRaRx,
        "TaskLoRaRx",
        4096,
        NULL,
        3,
        &xTaskLoRaRxHandle,
        1 // Core 1 (APP_CPU)
    );

    if (retProcessor == pdPASS && retRx == pdPASS) {
        Serial.println(F("[BOOT] TaskProcessor (Core 0) & TaskLoRaRx (Core 1) aktif!"));
        Serial.println(F("[BOOT] Node siap menerima paket LoRa di udara."));
    } else {
        Serial.println(F("[FATAL] Gagal membuat Task FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    Serial.println(F("========================================================================\n"));
}

// ===================================================================
// 9. LOOP ARDUINO
// ===================================================================
void loop() {
    vTaskDelete(NULL);
}
