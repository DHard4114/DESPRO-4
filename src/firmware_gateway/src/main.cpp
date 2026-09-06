#include <Arduino.h>

#define IS_GATEWAY
#ifdef IS_GATEWAY

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <RadioLib.h>

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
    float water_level_cm;
    float ammonia_ppm;
    float h2s_ppm;
    float battery_voltage;
    uint8_t sos_triggered;
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
    // Kalkulasi offset
    uint32_t offset = meta.tail * sizeof(TelemetryPayload);
    f.seek(offset, SeekSet);
    f.write((uint8_t*)&data, sizeof(TelemetryPayload));
    f.close();

    // Update pointers
    meta.tail = (meta.tail + 1) % MAX_BUFFER_SIZE;
    if (meta.count < MAX_BUFFER_SIZE) {
        meta.count++;
    } else {
        // Overwrite mode: head maju (data tertua hilang)
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
TaskHandle_t TaskLoRaRxHandle;
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, PIN_LORA_MISO);
WiFiClient espClient;
PubSubClient mqtt(espClient);

// ==========================================
// ISR: LORA DIO0 (Core 1)
// ==========================================
void IRAM_ATTR isr_lora_rx() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Beri sinyal ke Task LoRa bahwa paket siap dibaca
    vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// ==========================================
// TASK 1: LORA RX (Core 1, Prio 3)
// ==========================================
// DILARANG BLOKIR SAAT WIFI PUTUS!
void vTaskLoRaRx(void *pvParameters) {
    radio.setDio0Action(isr_sos_button); // akan direplace nanti oleh library
    // Kita panggil setDio0Action di setup
    for (;;) {
        // Block ringan menanti notifikasi ISR (Non-blocking ke OS)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TelemetryPayload rxData;
        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));

        if (state == RADIOLIB_ERR_NONE) {
            // Push ke antrean internal
            if (xQueueSend(xQueueTelemetry, &rxData, 0) != pdPASS) {
                // Queue penuh, langsung jatuhkan ke LittleFS
                pushToBuffer(rxData);
            }
        }
        
        // Kembalikan ke mode Listen
        radio.startReceive();
    }
}

// ==========================================
// TASK 2: MQTT TX & BUFFER FLUSH (Core 0, Prio 2)
// ==========================================
void vTaskMqttTx(void *pvParameters) {
    for (;;) {
        TelemetryPayload data;
        bool hasData = false;

        // Prioritas 1: Ambil dari Queue RAM
        if (xQueueReceive(xQueueTelemetry, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            hasData = true;
        } 
        // Prioritas 2: Ambil dari LittleFS jika MQTT konek
        else if (mqtt.connected() && meta.count > 0) {
            hasData = popFromBuffer(&data);
        }

        if (hasData) {
            if (mqtt.connected()) {
                // Konversi Biner -> JSON [ADR-01]
                StaticJsonDocument<256> doc;
                doc["schema_version"] = data.schema_version;
                doc["node_code"] = data.node_code;
                doc["sequence_no"] = data.sequence_no;
                doc["water_level_cm"] = data.water_level_cm;
                doc["ammonia_ppm"] = data.ammonia_ppm;
                doc["h2s_ppm"] = data.h2s_ppm;
                doc["battery_voltage"] = data.battery_voltage;
                doc["sos_triggered"] = data.sos_triggered;

                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);

                if (!mqtt.publish(MQTT_TOPIC_TX, jsonBuffer, true)) { // QoS 0 di ESP, Backend handle deduplikasi
                    // Jika gagal kirim (koneksi drop tiba-tiba), simpan ke buffer
                    pushToBuffer(data);
                }
            } else {
                // WiFi/MQTT Putus, simpan ke LittleFS
                pushToBuffer(data);
            }
        }
    }
}

// ==========================================
// TASK 3: WIFI & MQTT SUPERVISOR (Core 0, Prio 1)
// ==========================================
void vTaskWiFiSupervisor(void *pvParameters) {
    uint32_t backoff = 3000;
    
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi Terputus. Menghubungkan...");
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            
            // Tunggu hingga 5 detik
            int retries = 10;
            while (WiFi.status() != WL_CONNECTED && retries > 0) {
                vTaskDelay(pdMS_TO_TICKS(500));
                retries--;
            }
        }

        if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
            Serial.println("MQTT Terputus. Menghubungkan...");
            mqtt.setServer(MQTT_SERVER, MQTT_PORT);
            
            // LWT (Last Will and Testament) [Sesuai Dokumen Pipa Jaringan]
            if (mqtt.connect("Gateway_01", MQTT_USER, MQTT_PASS, "esos/gateway_01/status", 1, true, "OFFLINE")) {
                Serial.println("MQTT Terhubung!");
                mqtt.publish("esos/gateway_01/status", "ONLINE", true);
                backoff = 3000; // Reset backoff
            } else {
                Serial.printf("MQTT Gagal (rc=%d). Backoff %d ms\n", mqtt.state(), backoff);
                vTaskDelay(pdMS_TO_TICKS(backoff));
                if (backoff < 12000) backoff *= 2; // Exponential Backoff (3s, 6s, 12s)
            }
        }
        
        if (mqtt.connected()) {
            mqtt.loop();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Relieve CPU
    }
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    Serial.begin(115200);

    // Inisiasi Mutex & Queue
    fsMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(TelemetryPayload));

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
    xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi",  4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt",  4096, NULL, 2, NULL, 0);
    
    // Simpan Handle untuk Notify
    xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx", 4096, NULL, 3, &TaskLoRaRxHandle, 1);
}

void loop() {
    vTaskDelete(NULL);
}

#endif // IS_GATEWAY
