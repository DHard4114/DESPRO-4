#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <RadioLib.h>
#include <Wire.h>
#include <RTClib.h>
#include "config.h"

// ==========================================
// 1. OBJEK PERANGKAT KERAS & KERNEL FREERTOS
// ==========================================
// Argumen ke-4 Module adalah RADIOLIB_NC, oper objek SPI eksplisit
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);

RTC_DS3231 rtc;
WiFiClient espClient;
PubSubClient mqtt(espClient);

// Handle FreeRTOS
QueueHandle_t xQueueTelemetry = NULL;
QueueHandle_t xQueueCommandDownlink = NULL;
TaskHandle_t  TaskLoRaRxHandle = NULL;
SemaphoreHandle_t fsMutex = NULL;

// ==========================================
// 1B. LORAWAN CLASS A DOWNLINK QUEUE [ZERO-TRUST]
// ==========================================
struct PendingDownlink {
    ActuatorCommand cmd;
    bool active;
    uint32_t queued_at;
};

#define MAX_PENDING_NODES 8
static PendingDownlink pendingDownlinks[MAX_PENDING_NODES];
static SemaphoreHandle_t pendingMutex = NULL;

static void queuePendingDownlink(const ActuatorCommand &cmd) {
    if (pendingMutex == NULL) return;
    xSemaphoreTake(pendingMutex, portMAX_DELAY);
    int slot = -1;
    for (int i = 0; i < MAX_PENDING_NODES; i++) {
        if (pendingDownlinks[i].active && strncmp(pendingDownlinks[i].cmd.node_code, cmd.node_code, 8) == 0) {
            slot = i;
            break;
        } else if (!pendingDownlinks[i].active && slot == -1) {
            slot = i;
        }
    }
    if (slot != -1) {
        pendingDownlinks[slot].cmd = cmd;
        pendingDownlinks[slot].active = true;
        pendingDownlinks[slot].queued_at = (uint32_t)(millis() / 1000);
        Serial.printf("[CLASS-A QUEUE] Pending komando tersimpan untuk %s (CMD=%u, PARAM=%u)\n", 
                      cmd.node_code, cmd.command_id, cmd.parameter);
    }
    xSemaphoreGive(pendingMutex);
}

static bool popPendingDownlink(const char* node_code, ActuatorCommand *outCmd) {
    if (pendingMutex == NULL) return false;
    xSemaphoreTake(pendingMutex, portMAX_DELAY);
    bool found = false;
    for (int i = 0; i < MAX_PENDING_NODES; i++) {
        if (pendingDownlinks[i].active && strncmp(pendingDownlinks[i].cmd.node_code, node_code, 8) == 0) {
            *outCmd = pendingDownlinks[i].cmd;
            pendingDownlinks[i].active = false;
            found = true;
            Serial.printf("[CLASS-A DISPATCH] Komando pending diambil untuk %s (CMD=%u)\n", 
                          node_code, outCmd->command_id);
            break;
        }
    }
    xSemaphoreGive(pendingMutex);
    return found;
}

// ==========================================
// 2. LITTLEFS CIRCULAR BUFFER (Store-and-Forward) [ADR-03]
// ==========================================
#define MAX_BUFFER_SIZE 500
#define FILE_META "/meta.dat"
#define FILE_DATA "/buffer.dat"

struct BufferMeta {
    uint16_t head;
    uint16_t tail;
    uint16_t count;
};

static BufferMeta meta = {0, 0, 0};

static void saveMeta() {
    File f = LittleFS.open(FILE_META, FILE_WRITE);
    if (f) {
        f.write((uint8_t*)&meta, sizeof(BufferMeta));
        f.close();
    }
}

static void loadMeta() {
    if (LittleFS.exists(FILE_META)) {
        File f = LittleFS.open(FILE_META, FILE_READ);
        if (f) {
            f.read((uint8_t*)&meta, sizeof(BufferMeta));
            f.close();
        }
    } else {
        saveMeta();
    }
}

static void pushToBuffer(const TelemetryPayload &data) {
    if (fsMutex == NULL) return;
    xSemaphoreTake(fsMutex, portMAX_DELAY);
    
    File f = LittleFS.open(FILE_DATA, FILE_WRITE);
    if (f) {
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
        Serial.printf("[STORE-AND-FORWARD] Data disimpan ke Flash. Antrean tersimpan: %u\n", meta.count);
    }
    xSemaphoreGive(fsMutex);
}

static bool popFromBuffer(TelemetryPayload *data) {
    if (fsMutex == NULL || meta.count == 0) return false;
    xSemaphoreTake(fsMutex, portMAX_DELAY);

    bool success = false;
    File f = LittleFS.open(FILE_DATA, FILE_READ);
    if (f) {
        uint32_t offset = meta.head * sizeof(TelemetryPayload);
        f.seek(offset, SeekSet);
        f.read((uint8_t*)data, sizeof(TelemetryPayload));
        f.close();

        meta.head = (meta.head + 1) % MAX_BUFFER_SIZE;
        meta.count--;
        saveMeta();
        success = true;
        Serial.printf("[STORE-AND-FORWARD] Data di-pop dari Flash. Sisa antrean: %u\n", meta.count);
    }
    xSemaphoreGive(fsMutex);
    return success;
}

// ==========================================
// 3. ISR INTERRUPSI DIO0 (HARDWARE RX_DONE)
// ==========================================
void IRAM_ATTR isr_lora_rx() {
    if (TaskLoRaRxHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ==========================================
// 4. TASK: PENERIMA LORA RX ASINKRON (Core 1, Prio 3)
// ==========================================
void vTaskLoRaRx(void *pvParameters) {
    for (;;) {
        // Blokir sampai ada sinyal interupsi hardware DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TelemetryPayload rxData;
        memset(&rxData, 0, sizeof(TelemetryPayload));

        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));

        if (state == RADIOLIB_ERR_NONE) {
            // Berikan True Timestamp dari RTC DS3231 [ADR-03]
            rxData.timestamp = rtc.now().unixtime();
            
            Serial.printf("[LORA RX] Paket Diterima: Node=%s | Seq=#%u | Air=%.1f cm | RSSI=%.1f dBm | SNR=%.1f dB\n",
                          rxData.node_code, rxData.sequence_no, rxData.water_level_cm, 
                          radio.getRSSI(), radio.getSNR());

            // Kirim ke antrean telemetri. Jika antrean RAM penuh, simpan ke LittleFS
            if (xQueueSend(xQueueTelemetry, &rxData, 0) != pdPASS) {
                pushToBuffer(rxData);
            }

            // [LoRaWAN Class A Response]:
            // Node membuka RX Window 2000 ms tepat setelah transmit!
            // Cek apakah ada komando pending untuk node ini:
            ActuatorCommand pendingCmd;
            if (popPendingDownlink(rxData.node_code, &pendingCmd)) {
                vTaskDelay(pdMS_TO_TICKS(60)); // Beri waktu Node WC berganti ke mode RX
                radio.standby();
                Serial.printf("[CLASS-A DOWNLINK] Memancarkan Komando ke %s (CMD=%u, PARAM=%u) dalam RX Window!\n", 
                              pendingCmd.node_code, pendingCmd.command_id, pendingCmd.parameter);
                int txState = radio.transmit((uint8_t*)&pendingCmd, sizeof(ActuatorCommand));
                if (txState == RADIOLIB_ERR_NONE) {
                    Serial.println("[CLASS-A DOWNLINK] Komando Berhasil Dipancarkan ke RX Window Node!");
                } else {
                    Serial.printf("[CLASS-A DOWNLINK] Gagal Memancarkan Downlink: %d\n", txState);
                }
            }
        }

        // Segera aktifkan kembali mode RX Continuous
        radio.startReceive();
    }
}

// ==========================================
// 5. TASK: TRANSMISI LORA DOWNLINK COMMAND (Core 1, Prio 2)
// ==========================================
void vTaskLoRaTxDownlink(void *pvParameters) {
    ActuatorCommand cmd;
    for (;;) {
        if (xQueueReceive(xQueueCommandDownlink, &cmd, portMAX_DELAY) == pdTRUE) {
            radio.standby();
            Serial.printf("[DOWNLINK TX] Memancarkan Komando ke Node: %s (CMD: %u)...\n", cmd.node_code, cmd.command_id);
            int state = radio.transmit((uint8_t*)&cmd, sizeof(ActuatorCommand));
            
            if (state == RADIOLIB_ERR_NONE) {
                Serial.println("[DOWNLINK TX] Perintah Berhasil Dipancarkan ke Udara!");
            } else {
                Serial.printf("[DOWNLINK TX] Gagal Memancarkan Komando (Galat: %d)\n", state);
            }
            
            radio.startReceive();
        }
    }
}

// ==========================================
// 6. TASK: MQTT DISPATCH & STORE-AND-FORWARD (Core 0, Prio 2)
// ==========================================
void vTaskMqttTx(void *pvParameters) {
    for (;;) {
        TelemetryPayload data;
        bool hasData = false;

        // Cek apakah ada data di antrean RAM
        if (xQueueReceive(xQueueTelemetry, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            hasData = true;
        } 
        // Jika antrean RAM kosong, kuras data dari LittleFS saat MQTT online
        else if (mqtt.connected() && meta.count > 0) {
            hasData = popFromBuffer(&data);
        }

        if (hasData) {
            if (mqtt.connected()) {
                StaticJsonDocument<384> doc;
                doc["schema_version"]  = data.schema_version;
                doc["node_code"]       = data.node_code;
                doc["sequence_no"]     = data.sequence_no;
                doc["timestamp"]       = data.timestamp;
                doc["water_level_cm"]  = data.water_level_cm;
                doc["ammonia_ppm"]     = data.ammonia_ppm;
                doc["h2s_ppm"]         = data.h2s_ppm;
                doc["battery_voltage"] = data.battery_voltage;
                doc["sos_triggered"]   = data.sos_triggered;

                char jsonBuffer[384];
                serializeJson(doc, jsonBuffer);

                if (mqtt.publish(MQTT_TOPIC_TELEMETRY, jsonBuffer, true)) {
                    Serial.printf("[MQTT TX] Berhasil Publish ke %s: %s\n", MQTT_TOPIC_TELEMETRY, jsonBuffer);
                } else {
                    Serial.println("[MQTT TX] Publish gagal! Menyimpan ke buffer Flash.");
                    pushToBuffer(data);
                }
            } else {
                // Jaringan offline, amankan ke flash disk LittleFS
                pushToBuffer(data);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==========================================
// 7. TASK: SUPERVISOR WIFI & MQTT (Core 0, Prio 1)
// ==========================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (!error) {
        ActuatorCommand cmd;
        memset(&cmd, 0, sizeof(ActuatorCommand));
        const char* node = doc["node_code"] | "WC_01";
        strncpy(cmd.node_code, node, sizeof(cmd.node_code) - 1);
        cmd.command_id = doc["command_id"] | 1;
        cmd.parameter  = doc["parameter"] | 0;

        Serial.printf("[MQTT RX] Komando diterima dari server untuk %s: CMD=%u (Param=%u)\n", 
                      cmd.node_code, cmd.command_id, cmd.parameter);

        // [STRATEGI 1: LORAWAN CLASS A QUEUE]:
        // Simpan ke pending queue agar ditembakkan tepat saat Node WC menyapa di jendela RX 2000 ms
        queuePendingDownlink(cmd);

        // Tetap masukkan ke antrean direct downlink untuk fleksibilitas pengujian
        xQueueSend(xQueueCommandDownlink, &cmd, 0);
    }
}

void vTaskWiFiSupervisor(void *pvParameters) {
    uint32_t backoff = 3000;
    for (;;) {
        // Monitor Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            Serial.println("[WIFI] Menyambungkan ke WiFi Posko...");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

        // Monitor MQTT jika Wi-Fi sudah tersambung
        if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
            mqtt.setServer(MQTT_SERVER, MQTT_PORT);
            mqtt.setCallback(mqttCallback);

            String clientId = "GatewayPosko-" + String(WiFi.macAddress());
            Serial.printf("[MQTT] Menghubungi Broker %s:%d...\n", MQTT_SERVER, MQTT_PORT);

            if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
                Serial.println("[MQTT] Terhubung ke Broker!");
                mqtt.publish(MQTT_TOPIC_STATUS, "ONLINE", true);
                mqtt.subscribe(MQTT_TOPIC_COMMAND, 1);
                backoff = 3000;
            } else {
                Serial.printf("[MQTT] Gagal (rc=%d). Backoff %u ms\n", mqtt.state(), backoff);
                vTaskDelay(pdMS_TO_TICKS(backoff));
                if (backoff < 30000) backoff *= 2;
            }
        }

        if (mqtt.connected()) {
            mqtt.loop();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ==========================================
// 8. SETUP UTAMA GATEWAY
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=================================================================");
    Serial.println("  SMART-SANITATION eSOS - GATEWAY ROUTER FIRMWARE (ESP32 DevKit)");
    Serial.println("=================================================================");

    // Inisialisasi Bus I2C RTC DS3231 (SDA:4, SCL:22)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (!rtc.begin()) {
        Serial.println("[PERINGATAN] Modul RTC DS3231 tidak ditemukan pada I2C!");
    } else {
        Serial.println("[OK] Modul RTC DS3231 aktif.");
    }

    // Inisiasi Mutex & Queues
    fsMutex = xSemaphoreCreateMutex();
    pendingMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(TelemetryPayload));
    xQueueCommandDownlink = xQueueCreate(5, sizeof(ActuatorCommand));

    // Inisiasi LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[FATAL] LittleFS Mount Failed!");
    } else {
        loadMeta();
        Serial.printf("[OK] LittleFS aktif. Riwayat pesan tersimpan: %u\n", meta.count);
    }

    // Inisialisasi Hardware SPI Bus untuk SX1278
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);

    // Inisiasi Radio SX1278 via RadioLib
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        radio.setCRC(true);
        radio.setDio0Action(isr_lora_rx, RISING);
        radio.startReceive();
        Serial.println("[OK] Radio SX1278 aktif & Siaga di 433.175 MHz.");
    } else {
        Serial.printf("[GAGAL] Inisialisasi Radio SX1278 gagal (Kode: %d)\n", state);
    }

    // Pasang Task FreeRTOS ke Dual Core ESP32
    xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi",       4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt",       4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLoRaTxDownlink, "TaskLoRaTxDown", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx",     4096, NULL, 3, &TaskLoRaRxHandle, 1);

    Serial.println("=================================================================\n");
}

void loop() {
    vTaskDelete(NULL);
}
