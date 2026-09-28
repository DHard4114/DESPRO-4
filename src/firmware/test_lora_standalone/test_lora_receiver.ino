/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI MANDIRI PENERIMA (RECEIVER / GATEWAY): ESP32 DEVKIT V1 + Ai-Thinker Ra-02
 * ======================================================================================
 *
 * Fungsi Skrip:
 * 1. Menerima paket RF dari Node pemancar (test_lora_standalone.ino) pada 433.175 MHz.
 * 2. Memvalidasi panjang payload (tepat 34 bytes) dan integritas data biner.
 * 3. Mengukur metrik kekuatan sinyal riil: RSSI (dBm) dan SNR (dB).
 * 4. Mendeteksi packet loss berdasarkan diskontinuitas nomor urut (Sequence Number).
 *
 * Wiring Pin (Ai-Thinker Ra-02 -> ESP32 DevKit V1 Penerima):
 * - VCC  -> 3V3 (3.3V)
 * - GND  -> GND
 * - NSS  -> GPIO 5
 * - MOSI -> GPIO 23
 * - MISO -> GPIO 19
 * - SCK  -> GPIO 18
 * - RST  -> GPIO 14
 * - DIO0 -> GPIO 2 (Wajib untuk interupsi RX_DONE)
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// 1. PINOUT SPI Ra-02 ke ESP32
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18

// 2. PARAMETER RADIO RESMI (WAJIB PERSIS SAMA DENGAN TRANSMITTER)
#define LORA_FREQ         433.175 // Frekuensi tengah kanal legal (MHz)
#define LORA_BW           125.0   // Bandwidth kanal (kHz)
#define LORA_SF           9       // Spreading Factor
#define LORA_CR           7       // Coding Rate 4/7
#define LORA_SYNC_WORD    0x12    // Sync Word Jaringan Privat eSOS

// 3. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK (34 Bytes)
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

// Objek Hardware SX1278 (Argumen ke-4 adalah RADIOLIB_NC, bukan MISO)
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC);

// Statistik Penerimaan Paket
uint32_t total_received_packets = 0;
uint32_t last_sequence_no = 0;
uint32_t lost_packets_count = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n\n===================================================================");
    Serial.println("   SMART-SANITATION eSOS - UJI LORA RECEIVER (ESP32 DevKit V1)     ");
    Serial.println("===================================================================");
    Serial.println("Parameter Radio Penerima:");
    Serial.printf("  - Frekuensi       : %.3f MHz\n", LORA_FREQ);
    Serial.printf("  - Bandwidth       : %.1f kHz\n", LORA_BW);
    Serial.printf("  - Spreading Factor: SF%d\n", LORA_SF);
    Serial.printf("  - Coding Rate     : 4/%d\n", LORA_CR);
    Serial.printf("  - Sync Word       : 0x%02X\n", LORA_SYNC_WORD);
    Serial.println("-------------------------------------------------------------------");

    // 1. Inisialisasi Bus SPI
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);

    // 2. Inisialisasi Radio SX1278
    Serial.print("Menghubungi chip SX1278 Penerima via SPI... ");
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, 10);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[OK]");
        Serial.println(">>> MODUL PENERIMA AKTIF & MENDENGARKAN DI 433.175 MHz <<<");
        Serial.println("Menunggu paket transmisi dari Node...");
        Serial.println("===================================================================\n");
    } else {
        Serial.printf("[GAGAL] Kode Error: %d\n", state);
        Serial.println("Periksa jalur kabel SPI dan tegangan 3.3V.");
        while (true) delay(1000);
    }
}

void loop() {
    TelemetryPayload rxPayload;
    memset(&rxPayload, 0, sizeof(TelemetryPayload));

    // Terima paket secara sinkronus (blocking hingga paket tiba)
    int state = radio.receive((uint8_t*)&rxPayload, sizeof(TelemetryPayload));

    if (state == RADIOLIB_ERR_NONE) {
        total_received_packets++;

        // Ambil metrik kualitas sinyal RF aktual
        float rssi = radio.getRSSI();
        float snr = radio.getSNR();
        size_t packetLength = radio.getPacketLength();

        // Hitung packet loss berdasarkan sequence number
        if (last_sequence_no > 0 && rxPayload.sequence_no > last_sequence_no + 1) {
            uint32_t missed = rxPayload.sequence_no - (last_sequence_no + 1);
            lost_packets_count += missed;
            Serial.printf(">>> [PERINGATAN PACKET LOSS] Terdeteksi %u paket hilang antara Seq #%u dan #%u!\n",
                          missed, last_sequence_no, rxPayload.sequence_no);
        }
        last_sequence_no = rxPayload.sequence_no;

        float packetLossRate = 0.0f;
        if (rxPayload.sequence_no > 0) {
            packetLossRate = ((float)lost_packets_count / (float)rxPayload.sequence_no) * 100.0f;
        }

        Serial.println("===================================================================");
        Serial.printf(">>> [PAKET DITERIMA #%u] Panjang Biner: %u bytes (Validasi: %s)\n", 
                      total_received_packets, (unsigned int)packetLength, 
                      packetLength == sizeof(TelemetryPayload) ? "OK 34 Bytes" : "MISMATCH!");
        Serial.printf("    Kualitas Sinyal RF : RSSI = %.1f dBm | SNR = %.2f dB\n", rssi, snr);
        Serial.printf("    Statistik Drop     : Hilang = %u paket (Loss Rate: %.1f%%)\n", lost_packets_count, packetLossRate);
        Serial.println("-------------------------------------------------------------------");
        Serial.println("    Isi Data Ter-decode:");
        Serial.printf("      - Node ID       : %s\n", rxPayload.node_code);
        Serial.printf("      - Sequence No   : #%u\n", rxPayload.sequence_no);
        Serial.printf("      - Uptime Node   : %u detik\n", rxPayload.uptime_seconds);
        Serial.printf("      - Level Air     : %.1f cm\n", rxPayload.water_level_cm);
        Serial.printf("      - Gas Amonia    : %.1f ppm\n", rxPayload.ammonia_ppm);
        Serial.printf("      - Gas H2S       : %.1f ppm\n", rxPayload.h2s_ppm);
        Serial.printf("      - Voltase Baterai: %.2f V\n", rxPayload.battery_voltage);
        Serial.printf("      - Status SOS    : %s\n", rxPayload.sos_triggered ? "DARURAT (AKTIF)" : "Normal");
        Serial.println("===================================================================\n");

    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        // Timeout wajar jika belum ada transmisi
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
        Serial.println(">>> [ERROR] Paket diterima tetapi CRC Mismatch (Data Korup di Udara)!");
    } else {
        Serial.printf(">>> [ERROR RX] Kode galat penerimaan: %d\n", state);
    }
}
