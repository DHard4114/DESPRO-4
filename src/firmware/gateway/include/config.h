#ifndef CONFIG_GATEWAY_H
#define CONFIG_GATEWAY_H

#include <Arduino.h>

// ==========================================
// 1. KREDENSIAL JARINGAN & BROKER MQTT [ADR-01]
// ==========================================
// Konfigurasi Target Lapangan (CPE220 + Laptop Broker)
#define WIFI_SSID           "CompEngQuiz-Server-Live"
#define WIFI_PASS           "compengquiz"

// Konfigurasi Alamat IP Statis ESP32 Gateway (Subnet /24)
#define STATIC_IP_LOCAL     192, 168, 101, 11
#define STATIC_IP_GATEWAY   192, 168, 101, 1
#define STATIC_IP_SUBNET    255, 255, 255, 0
#define STATIC_IP_DNS       152, 118, 24, 4

// Konfigurasi Laptop / Broker Mosquitto MQTT
#define MQTT_SERVER         "192.168.101.10"
#define MQTT_PORT           1883
#define MQTT_USER           "esos_gateway"
#define MQTT_PASS           "gateway_secret"

// Topik MQTT Namespace eSOS
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
// Format biner 34-byte persis sesuai transmisi Node WC [ADR-01]
struct __attribute__((packed)) TelemetryPayload {
    uint8_t  schema_version; // Offset  0 | 1 Byte  : Versi skema biner (Selalu 1)
    char     node_code[8];   // Offset  1 | 8 Bytes : Identifier C-String ("WC_01\0\0\0")
    uint32_t sequence_no;    // Offset  9 | 4 Bytes : Nomor urut paket (Little-Endian)
    uint32_t uptime_seconds; // Offset 13 | 4 Bytes : Durasi aktif Node WC sejak boot (detik)
    float    water_level_cm; // Offset 17 | 4 Bytes : Ketinggian air tangki (IEEE-754)
    float    ammonia_ppm;    // Offset 21 | 4 Bytes : Konsentrasi gas amonia (IEEE-754)
    float    h2s_ppm;        // Offset 25 | 4 Bytes : Konsentrasi gas H2S (IEEE-754)
    float    battery_voltage;// Offset 29 | 4 Bytes : Tegangan baterai Li-ion (IEEE-754)
    uint8_t  sos_triggered;  // Offset 33 | 1 Byte  : Status darurat (0 = Normal, 1 = Darurat)
};

// Verifikasi compile-time ketat untuk ukuran dan seluruh offset field payload 34B
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");
static_assert(offsetof(TelemetryPayload, schema_version)  == 0,  "FATAL: Offset schema_version salah!");
static_assert(offsetof(TelemetryPayload, node_code)       == 1,  "FATAL: Offset node_code salah!");
static_assert(offsetof(TelemetryPayload, sequence_no)     == 9,  "FATAL: Offset sequence_no salah!");
static_assert(offsetof(TelemetryPayload, uptime_seconds)  == 13, "FATAL: Offset uptime_seconds salah!");
static_assert(offsetof(TelemetryPayload, water_level_cm)  == 17, "FATAL: Offset water_level_cm salah!");
static_assert(offsetof(TelemetryPayload, ammonia_ppm)     == 21, "FATAL: Offset ammonia_ppm salah!");
static_assert(offsetof(TelemetryPayload, h2s_ppm)         == 25, "FATAL: Offset h2s_ppm salah!");
static_assert(offsetof(TelemetryPayload, battery_voltage) == 29, "FATAL: Offset battery_voltage salah!");
static_assert(offsetof(TelemetryPayload, sos_triggered)   == 33, "FATAL: Offset sos_triggered salah!");

// Struktur Record Internal Gateway (Menyimpan payload asli + waktu penerimaan RTC Gateway terpisah)
struct __attribute__((packed)) GatewayTelemetryRecord {
    TelemetryPayload payload;           // Offset  0 | 34 Bytes : Payload asli tidak terdistorsi dari node
    uint32_t         gateway_timestamp; // Offset 34 |  4 Bytes : Unix Epoch dari RTC DS3231 saat paket diterima
};

static_assert(sizeof(GatewayTelemetryRecord) == 38, "FATAL: Ukuran GatewayTelemetryRecord harus tepat 38 bytes!");
static_assert(offsetof(GatewayTelemetryRecord, payload)           == 0,  "FATAL: Offset payload salah!");
static_assert(offsetof(GatewayTelemetryRecord, gateway_timestamp) == 34, "FATAL: Offset gateway_timestamp salah!");

// Struktur Komando Aktuator (Downlink 10 Bytes)
struct __attribute__((packed)) ActuatorCommand {
    char     node_code[8];   // Offset 0 | 8 Bytes : Node tujuan
    uint8_t  command_id;     // Offset 8 | 1 Byte  : 1 = LOCK_DOOR, 2 = UNLOCK_DOOR, 3 = FLUSH_EXHAUST
    uint8_t  parameter;      // Offset 9 | 1 Byte  : Sudut servo atau durasi detik
};
static_assert(sizeof(ActuatorCommand) == 10, "FATAL: Ukuran ActuatorCommand harus tepat 10 bytes!");
static_assert(offsetof(ActuatorCommand, node_code)  == 0, "FATAL: Offset node_code salah!");
static_assert(offsetof(ActuatorCommand, command_id) == 8, "FATAL: Offset command_id salah!");
static_assert(offsetof(ActuatorCommand, parameter)  == 9, "FATAL: Offset parameter salah!");

#endif // CONFIG_GATEWAY_H
