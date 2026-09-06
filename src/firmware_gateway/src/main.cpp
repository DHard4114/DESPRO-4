#include <Arduino.h>

#define IS_GATEWAY
#ifdef IS_GATEWAY

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <RadioLib.h>
#include "esp_pm.h"
#include "esp_sleep.h"
#include <Wire.h>
#include <RTClib.h>

// ==========================================
// KREDENSIAL & PARAMETER
// ==========================================
const char* WIFI_SSID     = "POSKO_WIFI";
const char* WIFI_PASS     = "12345678";
const char* MQTT_SERVER   = "192.168.0.100";
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "esos_gateway";
const char* MQTT_PASS     = "gateway_secret";
const char* MQTT_TOPIC_TX = "esos/gateway_01/nodes/telemetry";
const char* MQTT_TOPIC_RX = "esos/+/+/command";

// Parameter Radio (SX1278)
#define PIN_LORA_NSS      5
#define PIN_LORA_DIO0     2
#define PIN_LORA_RESET    14
#define PIN_LORA_MISO     19
#define PIN_LORA_MOSI     23
#define PIN_LORA_SCK      18
#define LORA_FREQ         433.0
#define LORA_BW           125.0
#define LORA_SF           9
#define LORA_CR           7
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     17

// ==========================================
// STRUKTUR DATA (REUSE FASE 3)
// ==========================================
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version;
    char node_code[8];
    uint32_t sequence_no;
    uint32_t timestamp;
    float water_level_cm;
    float ammonia_ppm;
    float h2s_ppm;
    float battery_voltage;
    uint8_t sos_triggered;
};

RTC_DS3231 rtc;

struct __attribute__((packed)) ActuatorCommand {
    uint8_t command_id; // 1 = OPEN, 2 = CLOSE, 3 = FLUSH
    uint8_t angle;
};

// ==========================================
// LITTLEFS CIRCULAR BUFFER (Store-and-Forward) [ADR-04]
// ==========================================
#define MAX_BUFFER_SIZE 500
#define FILE_META "/meta.dat"
#define FILE_DATA "/buffer.dat"

struct BufferMeta {
    uint16_t head;
    uint16_t tail;
    uint16_t count;
};

BufferMeta meta = {0, 0, 0};
SemaphoreHandle_t fsMutex;

void saveMeta() {
    File f = LittleFS.open(FILE_META, FILE_WRITE);
    if (f) {
        f.write((uint8_t*)&meta, sizeof(BufferMeta));
        f.close();
    }
}

void loadMeta() {
    if (LittleFS.exists(FILE_META)) {
        File f = LittleFS.open(FILE_META, FILE_READ);
        if (f) {
            f.read((uint8_t*)&meta, sizeof(BufferMeta));
            f.close();
        }
    } else {
        saveMeta(); // Inisialisasi awal
    }
}

void pushToBuffer(TelemetryPayload data) {
    xSemaphoreTake(fsMutex, portMAX_DELAY);
    
    File f = LittleFS.open(FILE_DATA, FILE_WRITE); // Buka mode tulis
    uint32_t offset = meta.tail * sizeof(TelemetryPayload);
    f.seek(offset, SeekSet);
    f.write((uint8_t*)&data, sizeof(TelemetryPayload));
    f.close();

    meta.tail = (meta.tail + 1) % MAX_BUFFER_SIZE;
    if (meta.count < MAX_BUFFER_SIZE) {
        meta.count++;
    } else {
        meta.head = (meta.head + 1) % MAX_BUFFER_SIZE;
    }
    saveMeta(); 
    
    xSemaphoreGive(fsMutex);
    Serial.printf("Buffer PUSH. Count: %d\n", meta.count);
}

bool popFromBuffer(TelemetryPayload* data) {
    xSemaphoreTake(fsMutex, portMAX_DELAY);
    if (meta.count == 0) {
        xSemaphoreGive(fsMutex);
        return false;
    }

    File f = LittleFS.open(FILE_DATA, FILE_READ);
    uint32_t offset = meta.head * sizeof(TelemetryPayload);
    f.seek(offset, SeekSet);
    f.read((uint8_t*)data, sizeof(TelemetryPayload));
    f.close();

    meta.head = (meta.head + 1) % MAX_BUFFER_SIZE;
    meta.count--;
    saveMeta();
    
    xSemaphoreGive(fsMutex);
    Serial.printf("Buffer POP. Count: %d\n", meta.count);
    return true;
}

// ==========================================
// GLOBAL HANDLES
// ==========================================
QueueHandle_t xQueueTelemetry;
QueueHandle_t xQueueCommandDownlink;
TaskHandle_t TaskLoRaRxHandle;
esp_pm_lock_handle_t active_lock;

SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, PIN_LORA_MISO);
WiFiClient espClient;
PubSubClient mqtt(espClient);

// ==========================================
// CALLBACK & ISR
// ==========================================
void IRAM_ATTR isr_lora_rx() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (strstr(topic, "time_sync") != nullptr) {
        String ts = "";
        for (int i = 0; i < length; i++) ts += (char)payload[i];
        rtc.adjust(DateTime(ts.toInt()));
        Serial.printf("RTC Synced: %s\n", ts.c_str());
        return;
    }

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) return;

    ActuatorCommand cmd;
    const char* c_type = doc["command_type"];
    
    if (c_type != nullptr) {
        if (strcmp(c_type, "OPEN_VALVE") == 0) cmd.command_id = 1;
        else if (strcmp(c_type, "CLOSE_VALVE") == 0) cmd.command_id = 2;
        else if (strcmp(c_type, "FLUSH_TANK") == 0) cmd.command_id = 3;
        else return;
        
        cmd.angle = doc["target_angle_deg"] | 0;

        xQueueSend(xQueueCommandDownlink, &cmd, 0);
    }
}

// ==========================================
// TASK 1: LORA RX (Core 1, Prio 3)
// ==========================================
void vTaskLoRaRx(void *pvParameters) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TelemetryPayload rxData;
        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));

        if (state == RADIOLIB_ERR_NONE) {
            rxData.timestamp = rtc.now().unixtime();
            
            if (xQueueSend(xQueueTelemetry, &rxData, 0) != pdPASS) {
                pushToBuffer(rxData);
            }
        }
        
        radio.startReceive();
    }
}

// ==========================================
// TASK 2: LORA TX DOWNLINK (Core 1, Prio 2)
// ==========================================
void vTaskLoRaTxDownlink(void *pvParameters) {
    ActuatorCommand cmd;
    for (;;) {
        if (xQueueReceive(xQueueCommandDownlink, &cmd, portMAX_DELAY) == pdTRUE) {
            // Ambil Power Lock agar stabil
            esp_pm_lock_acquire(active_lock);
            
            radio.standby(); // Stop Receive mode

            int state = radio.transmit((uint8_t*)&cmd, sizeof(ActuatorCommand));
            if (state == RADIOLIB_ERR_NONE) {
                Serial.println("LoRa Downlink (Command) Terkirim!");
            } else {
                Serial.printf("LoRa Downlink Gagal (rc=%d)\n", state);
            }
            
            radio.startReceive(); // Kembali Listen
            
            // Lepas Power Lock
            esp_pm_lock_release(active_lock);
        }
    }
}

// ==========================================
// TASK 3: MQTT TX & BUFFER FLUSH (Core 0, Prio 2)
// ==========================================
void vTaskMqttTx(void *pvParameters) {
    for (;;) {
        TelemetryPayload data;
        bool hasData = false;

        if (xQueueReceive(xQueueTelemetry, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            hasData = true;
        } 
        else if (mqtt.connected() && meta.count > 0) {
            hasData = popFromBuffer(&data);
        }

        if (hasData) {
            if (mqtt.connected()) {
                StaticJsonDocument<256> doc;
                doc["schema_version"] = data.schema_version;
                doc["node_code"] = data.node_code;
                doc["sequence_no"] = data.sequence_no;
                doc["timestamp"] = data.timestamp;
                doc["water_level_cm"] = data.water_level_cm;
                doc["ammonia_ppm"] = data.ammonia_ppm;
                doc["h2s_ppm"] = data.h2s_ppm;
                doc["battery_voltage"] = data.battery_voltage;
                doc["sos_triggered"] = data.sos_triggered;

                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);

                if (!mqtt.publish(MQTT_TOPIC_TX, jsonBuffer, true)) {
                    pushToBuffer(data);
                }
            } else {
                pushToBuffer(data);
            }
        }
    }
}

// ==========================================
// TASK 4: WIFI & MQTT SUPERVISOR (Core 0, Prio 1)
// ==========================================
void vTaskWiFiSupervisor(void *pvParameters) {
    uint32_t backoff = 3000;
    
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi Terputus. Menghubungkan...");
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            
            int retries = 10;
            while (WiFi.status() != WL_CONNECTED && retries > 0) {
                vTaskDelay(pdMS_TO_TICKS(500));
                retries--;
            }
        }

        if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
            Serial.println("MQTT Terputus. Menghubungkan...");
            mqtt.setServer(MQTT_SERVER, MQTT_PORT);
            mqtt.setCallback(mqttCallback);
            
            if (mqtt.connect("Gateway_01", MQTT_USER, MQTT_PASS, "esos/gateway_01/status", 1, true, "OFFLINE")) {
                Serial.println("MQTT Terhubung!");
                mqtt.publish("esos/gateway_01/status", "ONLINE", true);
                mqtt.subscribe(MQTT_TOPIC_RX, 1);
                mqtt.subscribe("esos/gateway_01/time_sync", 1);
                backoff = 3000;
            } else {
                Serial.printf("MQTT Gagal (rc=%d). Backoff %d ms\n", mqtt.state(), backoff);
                vTaskDelay(pdMS_TO_TICKS(backoff));
                if (backoff < 12000) backoff *= 2; 
            }
        }
        
        if (mqtt.connected()) {
            mqtt.loop();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    Serial.begin(115200);

    Wire.begin(21, 22);
    if (!rtc.begin()) {
        Serial.println("RTC tidak ditemukan");
    }

    // Power Management
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "active_lock", &active_lock);

    // Inisiasi Mutex & Queues
    fsMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(TelemetryPayload));
    xQueueCommandDownlink = xQueueCreate(5, sizeof(ActuatorCommand));

    // Inisiasi LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    loadMeta();

    // Inisiasi LoRa
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        radio.setDio0Action(isr_lora_rx);
        radio.startReceive();
    } else {
        Serial.println("LoRa INIT FAILED!");
    }

    // Pembuatan Task
    xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi",       4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt",       4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLoRaTxDownlink, "TaskLoRaTxDown", 4096, NULL, 2, NULL, 1);
    
    // Simpan Handle untuk Notify (ISR)
    xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx",     4096, NULL, 3, &TaskLoRaRxHandle, 1);
}

void loop() {
    vTaskDelete(NULL);
}

#endif // IS_GATEWAY
