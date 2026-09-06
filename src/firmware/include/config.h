#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. KONTRAK PAYLOAD BINER MUTLAK (Max 50 Bytes)
// ==========================================
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

// ==========================================
// 2. IDENTITAS NODE & PARAMETER LORA
// ==========================================
#ifdef IS_NODE_WC
    #define NODE_ID "WC_01"  // Ubah sesuai Node fisik (WC_02, WC_03)
#else
    #define NODE_ID "GATEWAY"
#endif

// Parameter Radio (SX1278)
#define LORA_FREQ         433.0
#define LORA_BW           125.0
#define LORA_SF           9
#define LORA_CR           7
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     17

// ==========================================
// 3. DEFINISI PIN GPIO (ESP32)
// ==========================================

// Pin Radio SX1278 (SPI)
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18

// Pin Sensor Node WC
#define PIN_TRIG_US       13  // Ultrasonik
#define PIN_ECHO_US       12  // Ultrasonik
#define PIN_MQ137_AO      32  // Analog Ammonia
#define PIN_MQ136_AO      33  // Analog H2S
#define PIN_BTN_SOS       27  // Eksternal Interrupt (EXTI)
#define PIN_SERVO         26  // PWM Servo MG996R
#define PIN_BATT_VOLT     34  // Analog Battery Divider

// ==========================================
// 4. KREDENSIAL JARINGAN (HANYA GATEWAY)
// ==========================================
#ifdef IS_GATEWAY
    #define WIFI_SSID       "Posko_Sanitasi_AP"
    #define WIFI_PASS       "poskoadmin123"
    #define MQTT_BROKER_IP  "192.168.0.100"
    #define MQTT_PORT       1883
    #define MQTT_CLIENT_ID  "Gateway_ESP32_01"
    
    // Topik MQTT
    #define TOPIC_TELEMETRY "esos/posko-a/telemetry/ingest"
    #define TOPIC_COMMAND   "esos/posko-a/+/command"
#endif

#endif // CONFIG_H
