#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#ifdef IS_NODE_WC

// ==========================================
// 1. IDENTITAS NODE & PARAMETER LORA
// ==========================================
#define NODE_CODE "WC_01"  // Identitas sentral [ADR-06]

// Parameter Radio (SX1278 - Ai-Thinker Ra-02)
// Sesuai Regulasi Kominfo No. 2/2019 (Pita LPWAN/SRD Indonesia: 433,050 - 434,790 MHz)
// Frekuensi 433.175 MHz dengan BW 125 kHz menempati kanal 433,1125 - 433,2375 MHz (100% legal)
#define LORA_FREQ         433.175
#define LORA_BW           125.0
#define LORA_SF           9
#define LORA_CR           7
#define LORA_SYNC_WORD    0x12

// Batas regulasi EIRP Indonesia: Maks 12,15 dBm (10 dBm ERP / 10 mW)
// Conducted 10 dBm + Antena 2 dBi = 12 dBm EIRP <= 12,15 dBm EIRP
#define LORA_TX_POWER     10      // Default patuh regulasi lapangan (dBm)
#define LORA_TX_POWER_MAX 17      // Opsi lab berpelindung / atenuator (+17 dBm PA_BOOST)

// ==========================================
// 2. DEFINISI PIN GPIO (ESP32 DevKit V1)
// ==========================================
// Pin Radio SX1278 (Hardware SPI)
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2       // Strapping pin / Onboard LED (Pindah ke GPIO 4 jika boot terganggu)
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18

// Pin Sensor & Aktuator
#define PIN_TRIG_US       13
#define PIN_ECHO_US       12
#define PIN_MQ137_AO      32
#define PIN_MQ136_AO      33
#define PIN_BTN_SOS       27
#define PIN_SERVO         26
#define PIN_BATT_VOLT     34      // ADC1_CH6 (Input-only pin pada DevKit V1)

// ==========================================
// 3. KONTRAK PAYLOAD BINER MUTLAK
// ==========================================
// Format: Little-Endian (Xtensa LX6), Ukuran tepat 34 Bytes [ADR-01]
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Versi skema biner (Selalu 1)
    char node_code[8];      // Offset  1 | 8 bytes : "WC_01\0\0\0"
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket (Little-Endian)
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu sejak boot (Detik, bukan Unix epoch)
    float water_level_cm;   // Offset 17 | 4 bytes : Level air (IEEE-754 Little-Endian)
    float ammonia_ppm;      // Offset 21 | 4 bytes : Amonia (IEEE-754 Little-Endian)
    float h2s_ppm;          // Offset 25 | 4 bytes : H2S (IEEE-754 Little-Endian)
    float battery_voltage;  // Offset 29 | 4 bytes : Tegangan baterai 18650 (IEEE-754 Little-Endian)
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : 1 = Darurat, 0 = Normal
};

// Verifikasi compile-time ukuran struct
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

struct __attribute__((packed)) ActuatorCommand {
    uint8_t command_id; // 1 = OPEN, 2 = CLOSE, 3 = FLUSH
    uint8_t angle;
};
static_assert(sizeof(ActuatorCommand) == 2, "FATAL: Ukuran ActuatorCommand harus tepat 2 bytes!");

#endif // IS_NODE_WC

#endif // CONFIG_H
