#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#ifdef IS_NODE_WC

// ==========================================
// 1. IDENTITAS NODE & PARAMETER LORA
// ==========================================
#define NODE_CODE "WC_01"  // Identitas sentral [ADR-06]

// Parameter Radio (SX1278)
#define LORA_FREQ         433.0
#define LORA_BW           125.0
#define LORA_SF           9
#define LORA_CR           7
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     17

// ==========================================
// 2. DEFINISI PIN GPIO (ESP32)
// ==========================================
// Pin Radio SX1278 (SPI)
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2
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
#define PIN_BATT_VOLT     34

// ==========================================
// 3. KONTRAK PAYLOAD BINER MUTLAK
// ==========================================
// DILARANG MENGGUNAKAN JSON [ADR-01]
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Selalu 1
    char node_code[8];      // "WC_01"
    uint32_t sequence_no;
    uint32_t timestamp;
    float water_level_cm;
    float ammonia_ppm;
    float h2s_ppm;
    float battery_voltage;
    uint8_t sos_triggered;  // 1 = True, 0 = False
};

struct __attribute__((packed)) ActuatorCommand {
    uint8_t command_id; // 1 = OPEN, 2 = CLOSE, 3 = FLUSH
    uint8_t angle;
};

#endif // IS_NODE_WC

#endif // CONFIG_H
