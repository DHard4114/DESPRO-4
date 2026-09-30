#ifndef CONFIG_GATEWAY_H
#define CONFIG_GATEWAY_H

#include <Arduino.h>

// ==========================================
// 1. KREDENSIAL JARINGAN & BROKER MQTT [ADR-01]
// ==========================================
#define WIFI_SSID           "POSKO_WIFI"
#define WIFI_PASS           "12345678"
#define MQTT_SERVER         "192.168.0.100"
#define MQTT_PORT           1883
#define MQTT_USER           "esos_gateway"
#define MQTT_PASS           "gateway_secret"

#define MQTT_TOPIC_TELEMETRY "esos/gateway_01/nodes/telemetry"
#define MQTT_TOPIC_COMMAND   "esos/+/+/command"
#define MQTT_TOPIC_STATUS    "esos/gateway_01/status"
#define MQTT_TOPIC_TIMESYNC  "esos/gateway_01/time_sync"

// ==========================================
// 2. PARAMETER LORA (Ai-Thinker Ra-02 / SX1278)
// ==========================================
// Rujukan Regulasi: Permenkomdigi No. 2 Tahun 2025
#define LORA_FREQ           433.175 // Frekuensi tengah legal (MHz)
#define LORA_BW             125.0   // Bandwidth kanal (kHz)
#define LORA_SF             9       // Spreading Factor
#define LORA_CR             7       // Coding Rate 4/7
#define LORA_SYNC_WORD      0x12    // Sync Word Jaringan Privat eSOS
#define LORA_TX_POWER       10      // Conducted Power (dBm) - Target EIRP <= 12,15 dBm

// ==========================================
// 3. DEFINISI PIN PERANGKAT KERAS (ESP32 DevKit V1)
// ==========================================
// Bus SPI Radio LoRa (Jumper Fisik Aktif Terverifikasi)
#define PIN_LORA_SCK        21      // D21 ESP32 (Cokelat)
#define PIN_LORA_MISO       19      // D19 ESP32 (Merah)
#define PIN_LORA_MOSI       18      // D18 ESP32 (Oranye)
#define PIN_LORA_NSS        5       // D5  ESP32 (Kuning)
#define PIN_LORA_DIO0       2       // D2  ESP32 (Ungu) - Interupsi RX/TX Done
#define PIN_LORA_RESET      15      // D15 ESP32 (Biru) - Reset Hardware

// Bus I2C RTC DS3231 (GPIO 4 & 22 agar tidak bentrok dengan SCK di GPIO 21)
#define PIN_I2C_SDA         4       // D4  ESP32
#define PIN_I2C_SCL         22      // D22 ESP32

// ==========================================
// 4. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK
// ==========================================
struct __attribute__((packed)) TelemetryPayload {
    uint8_t  schema_version; // Offset  0 | 1 Byte  : Versi skema biner (Selalu 1)
    char     node_code[8];   // Offset  1 | 8 Bytes : Identifier C-String ("WC_01\0\0\0")
    uint32_t sequence_no;    // Offset  9 | 4 Bytes : Nomor urut paket
    uint32_t timestamp;      // Offset 13 | 4 Bytes : Unix Epoch Time (Diisi Gateway dari RTC)
    float    water_level_cm; // Offset 17 | 4 Bytes : Ketinggian air tangki
    float    ammonia_ppm;    // Offset 21 | 4 Bytes : Konsentrasi gas amonia
    float    h2s_ppm;        // Offset 25 | 4 Bytes : Konsentrasi gas H2S
    float    battery_voltage;// Offset 29 | 4 Bytes : Tegangan baterai Li-ion
    uint8_t  sos_triggered;  // Offset 33 | 1 Byte  : Status darurat (0 = Normal, 1 = Darurat)
};

static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// Struktur Komando Aktuator (Downlink)
struct __attribute__((packed)) ActuatorCommand {
    char     node_code[8];   // Node tujuan
    uint8_t  command_id;     // 1 = LOCK_DOOR, 2 = UNLOCK_DOOR, 3 = FLUSH_EXHAUST
    uint8_t  parameter;      // Sudut servo atau durasi detik
};

#endif // CONFIG_GATEWAY_H
