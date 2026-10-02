#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <RadioLib.h>
#include <Wire.h>
#include <RTClib.h>
#include <ctype.h>
#include "config.h"

// =========================================================================================
// SMART-SANITATION eSOS — GATEWAY ROUTER FIRMWARE (ESP32 DevKit V1)
// Project      : Desain Proyek 2 (Kelompok 4 FTUI - Gasal 2026/2027)
// Framework    : Arduino-ESP32 / ESP-IDF FreeRTOS (Dual-Core Xtensa LX6)
// Hardware     : DOIT ESP32 DevKit V1, SX1278 (Ai-Thinker Ra-02), DS3231 RTC
// Architecture : Single Radio Owner (TaskLoRaRx), Two-Phase Store-and-Forward (LittleFS "r+"),
//                Strict Uplink-Only Alignment, Thread-Safe MQTT Mutex, Non-truncating Buffer,
//                Static IP (192.168.101.11), Dedicated Server Time-Sync Handler.
// =========================================================================================

// ==========================================
// 0. FEATURE FLAGS & CONFIGURATION
// ==========================================
// Tahap operasional saat ini: Uplink-Only.
// Node WC beroperasi transmit-only dan langsung standby (tidak membuka RX window).
// Gateway dinonaktifkan dari memancarkan downlink agar radio tetap siaga RX kontinu.
#define ENABLE_GATEWAY_DOWNLINK     0   // 0 = Nonaktif (Uplink-Only), 1 = Mode Pengembangan

// ==========================================
// 1. OBJEK PERANGKAT KERAS & KERNEL FREERTOS
// ==========================================
// Radio SX1278 (Hardware SPI Bus). Argumen ke-4 adalah RADIOLIB_NC, SPI dioper eksplisit
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);

RTC_DS3231 rtc;
static bool g_rtc_available = false;
static bool g_radio_available = false;
static bool g_fs_available = false;

WiFiClient espClient;
PubSubClient mqtt(espClient);

// Handle FreeRTOS
QueueHandle_t xQueueTelemetry = NULL;
TaskHandle_t  TaskLoRaRxHandle = NULL;
SemaphoreHandle_t fsMutex = NULL;
SemaphoreHandle_t mqttMutex = NULL;

// ==========================================
// 2. TWO-PHASE STORE-AND-FORWARD CIRCULAR BUFFER (LittleFS)
// ==========================================
// Kebijakan Buffer Penuh: OVERWRITE_OLDEST (Drop slot tertua di head jika buffer penuh 500 record).
// Penjaminan: At-Least-Once Delivery. Record dibaca via peekBuffer(), dipublish ke MQTT,
// lalu dihapus via commitBufferDeletion(). Jika publish gagal, record tetap berada di head.
#define MAX_BUFFER_RECORDS  500
#define FILE_META           "/meta.dat"
#define FILE_DATA           "/buffer.dat"
#define BUFFER_MAGIC        0x47574233  // 'GWB3' (Schema v3: GatewayTelemetryRecord 38B)
#define BUFFER_VERSION      3

struct __attribute__((packed)) BufferMeta {
    uint32_t magic;
    uint16_t version;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint32_t dropped_count;
    uint32_t head_seq;      // Nomor urut paket pada posisi head untuk validasi two-phase commit
    uint32_t crc32;
};

struct CommitToken {
    uint16_t slot;
    uint32_t sequence_no;
    bool valid;
};

static BufferMeta meta = {BUFFER_MAGIC, BUFFER_VERSION, 0, 0, 0, 0, 0, 0};

static uint32_t calculateMetaCRC(const BufferMeta &m) {
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t *data = (const uint8_t*)&m;
    size_t length = offsetof(BufferMeta, crc32);
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

static bool saveMeta() {
    meta.magic = BUFFER_MAGIC;
    meta.version = BUFFER_VERSION;
    meta.crc32 = calculateMetaCRC(meta);

    File f = LittleFS.open(FILE_META, "w");
    if (!f) {
        Serial.println(F("[LITTLEFS ERROR] Gagal membuka FILE_META untuk penulisan!"));
        return false;
    }
    size_t written = f.write((const uint8_t*)&meta, sizeof(BufferMeta));
    f.flush();
    f.close();

    if (written != sizeof(BufferMeta)) {
        Serial.printf("[LITTLEFS ERROR] Gagal menulis seluruh metadata (%u != %u)\n",
                      (unsigned int)written, (unsigned int)sizeof(BufferMeta));
        return false;
    }
    return true;
}

static void initDataFileIfMissing() {
    if (!LittleFS.exists(FILE_DATA)) {
        File f = LittleFS.open(FILE_DATA, "w");
        if (f) {
            f.close();
            Serial.println(F("[LITTLEFS] Berkas buffer.dat baru berhasil dibuat."));
        } else {
            Serial.println(F("[LITTLEFS ERROR] Gagal membuat buffer.dat baru!"));
        }
    }
}

static void loadMeta() {
    initDataFileIfMissing();
    bool valid = false;

    if (LittleFS.exists(FILE_META)) {
        File f = LittleFS.open(FILE_META, "r");
        if (f) {
            BufferMeta temp;
            if (f.read((uint8_t*)&temp, sizeof(BufferMeta)) == sizeof(BufferMeta)) {
                if (temp.magic == BUFFER_MAGIC && temp.version == BUFFER_VERSION &&
                    temp.crc32 == calculateMetaCRC(temp) &&
                    temp.head < MAX_BUFFER_RECORDS && temp.tail < MAX_BUFFER_RECORDS &&
                    temp.count <= MAX_BUFFER_RECORDS) {
                    
                    // Verifikasi konsistensi ukuran berkas data dengan metadata
                    File dataFile = LittleFS.open(FILE_DATA, "r");
                    if (dataFile) {
                        size_t fileSize = dataFile.size();
                        dataFile.close();
                        if (temp.count == 0 || fileSize >= (size_t)temp.count * sizeof(GatewayTelemetryRecord)) {
                            meta = temp;
                            valid = true;
                        } else {
                            Serial.printf("[LITTLEFS ERROR] Ukuran berkas buffer.dat (%u B) tidak mencukupi untuk %u record!\n",
                                          (unsigned int)fileSize, (unsigned int)temp.count);
                        }
                    }
                } else {
                    Serial.println(F("[LITTLEFS ERROR] Metadata buffer.dat korup atau CRC32 mismatch!"));
                }
            } else {
                Serial.println(F("[LITTLEFS ERROR] Pembacaan metadata terpotong (short read)!"));
            }
            f.close();
        } else {
            Serial.println(F("[LITTLEFS ERROR] Gagal membuka FILE_META untuk pembacaan!"));
        }
    } else {
        Serial.println(F("[LITTLEFS NOTICE] FILE_META belum ada (sistem baru atau belum diinisialisasi)."));
    }

    if (!valid) {
        Serial.println(F("[LITTLEFS] Menginisialisasi metadata baru (Schema v3)..."));
        meta.magic = BUFFER_MAGIC;
        meta.version = BUFFER_VERSION;
        meta.head = 0;
        meta.tail = 0;
        meta.count = 0;
        meta.dropped_count = 0;
        meta.head_seq = 0;
        saveMeta();
    }
}

// Push data ke flash buffer tanpa pemotongan ("r+" mode)
static bool pushToBuffer(const GatewayTelemetryRecord &record) {
    if (fsMutex == NULL || !g_fs_available) return false;
    xSemaphoreTake(fsMutex, portMAX_DELAY);

    bool success = false;
    if (!LittleFS.exists(FILE_DATA)) {
        initDataFileIfMissing();
    }

    // Buka mode "r+" untuk update posisi acak tanpa truncating berkas lama
    File f = LittleFS.open(FILE_DATA, "r+");
    if (!f) {
        Serial.println(F("[LITTLEFS ERROR] Gagal membuka buffer.dat dalam mode r+ (I/O error, berkas dipertahankan)!"));
        xSemaphoreGive(fsMutex);
        return false;
    }

    uint32_t slot = meta.tail;
    uint32_t offset = slot * sizeof(GatewayTelemetryRecord);
    if (f.seek(offset, SeekSet)) {
        size_t written = f.write((const uint8_t*)&record, sizeof(GatewayTelemetryRecord));
        f.flush();
        if (written == sizeof(GatewayTelemetryRecord)) {
            if (meta.count == 0) {
                meta.head_seq = record.payload.sequence_no;
            }
            meta.tail = (meta.tail + 1) % MAX_BUFFER_RECORDS;
            if (meta.count < MAX_BUFFER_RECORDS) {
                meta.count++;
            } else {
                // Buffer penuh: timpa slot tertua di head (Kebijakan: OVERWRITE_OLDEST)
                meta.head = (meta.head + 1) % MAX_BUFFER_RECORDS;
                meta.dropped_count++;
                Serial.printf("[STORE-AND-FORWARD ALERT] Buffer penuh (%u rekaman). Slot tertua ditimpa! Total dropped: %u\n",
                              MAX_BUFFER_RECORDS, meta.dropped_count);
                // Baca nomor urut record baru yang kini berada di posisi head
                GatewayTelemetryRecord newHead;
                if (f.seek(meta.head * sizeof(GatewayTelemetryRecord), SeekSet)) {
                    if (f.read((uint8_t*)&newHead, sizeof(GatewayTelemetryRecord)) == sizeof(GatewayTelemetryRecord)) {
                        meta.head_seq = newHead.payload.sequence_no;
                    }
                }
            }
            saveMeta();
            success = true;
            Serial.printf("[STORE-AND-FORWARD] Data disimpan ke LittleFS (Slot %u, Antrean: %u, Seq: #%u)\n",
                          slot, meta.count, record.payload.sequence_no);
        } else {
            Serial.println(F("[LITTLEFS ERROR] Penulisan payload tidak lengkap ke flash!"));
        }
    } else {
        Serial.println(F("[LITTLEFS ERROR] Seek gagal pada pushToBuffer!"));
    }
    f.close();

    xSemaphoreGive(fsMutex);
    return success;
}

// Tahap 1 Two-Phase Commit: Peek data pada posisi head tanpa menghapusnya
static bool peekBuffer(GatewayTelemetryRecord *outRecord, CommitToken *outToken) {
    if (fsMutex == NULL || !g_fs_available || meta.count == 0 || outRecord == NULL || outToken == NULL) {
        return false;
    }
    xSemaphoreTake(fsMutex, portMAX_DELAY);

    bool success = false;
    outToken->valid = false;

    File f = LittleFS.open(FILE_DATA, "r");
    if (f) {
        uint32_t slot = meta.head;
        uint32_t offset = slot * sizeof(GatewayTelemetryRecord);
        if (f.seek(offset, SeekSet)) {
            if (f.read((uint8_t*)outRecord, sizeof(GatewayTelemetryRecord)) == sizeof(GatewayTelemetryRecord)) {
                outToken->slot = (uint16_t)slot;
                outToken->sequence_no = outRecord->payload.sequence_no;
                outToken->valid = true;
                success = true;
            } else {
                Serial.println(F("[LITTLEFS ERROR] Short read saat peekBuffer!"));
            }
        }
        f.close();
    } else {
        Serial.println(F("[LITTLEFS ERROR] Gagal membuka buffer.dat untuk peekBuffer!"));
    }

    xSemaphoreGive(fsMutex);
    return success;
}

// Tahap 2 Two-Phase Commit: Hapus logis record setelah MQTT publish berhasil
static bool commitBufferDeletion(const CommitToken &token) {
    if (fsMutex == NULL || !g_fs_available || !token.valid || meta.count == 0) {
        return false;
    }
    xSemaphoreTake(fsMutex, portMAX_DELAY);

    bool success = false;
    // Verifikasi bahwa posisi head belum berubah atau ditimpa oleh wrap-around
    if (meta.head == token.slot && meta.head_seq == token.sequence_no) {
        meta.head = (meta.head + 1) % MAX_BUFFER_RECORDS;
        meta.count--;
        
        // Perbarui head_seq jika masih ada sisa antrean
        if (meta.count > 0) {
            File f = LittleFS.open(FILE_DATA, "r");
            if (f) {
                GatewayTelemetryRecord nextHead;
                if (f.seek(meta.head * sizeof(GatewayTelemetryRecord), SeekSet)) {
                    if (f.read((uint8_t*)&nextHead, sizeof(GatewayTelemetryRecord)) == sizeof(GatewayTelemetryRecord)) {
                        meta.head_seq = nextHead.payload.sequence_no;
                    }
                }
                f.close();
            }
        }

        saveMeta();
        success = true;
        Serial.printf("[STORE-AND-FORWARD] Commit selesai. Data terkirim di-pop dari Flash. Sisa: %u (Seq: #%u)\n",
                      meta.count, token.sequence_no);
    } else {
        Serial.printf("[STORE-AND-FORWARD NOTICE] Commit dilewati: Slot head telah bergeser (%u != %u) atau seq mismatch (%u != %u). Data telah ditimpa wrap-around.\n",
                      meta.head, token.slot, meta.head_seq, token.sequence_no);
    }

    xSemaphoreGive(fsMutex);
    return success;
}

// ==========================================
// 3. VALIDASI TELEMETRI LORA & KONTRAK DATA
// ==========================================
// Helper validasi float dan batas fisis payload biner
static bool validateTelemetryFloats(const TelemetryPayload &p, char* err_buf, size_t err_len) {
    if (isnan(p.water_level_cm) || isinf(p.water_level_cm)) {
        snprintf(err_buf, err_len, "water_level_cm bernilai NaN/Inf");
        return false;
    }
    if (isnan(p.ammonia_ppm) || isinf(p.ammonia_ppm)) {
        snprintf(err_buf, err_len, "ammonia_ppm bernilai NaN/Inf");
        return false;
    }
    if (isnan(p.h2s_ppm) || isinf(p.h2s_ppm)) {
        snprintf(err_buf, err_len, "h2s_ppm bernilai NaN/Inf");
        return false;
    }
    if (isnan(p.battery_voltage) || isinf(p.battery_voltage)) {
        snprintf(err_buf, err_len, "battery_voltage bernilai NaN/Inf");
        return false;
    }

    // Pengecekan batas rentang nilai (mempertahankan sentinel -1.0f untuk nilai belum terkalibrasi/tersedia)
    if (p.water_level_cm != -1.0f && (p.water_level_cm < 0.0f || p.water_level_cm > 1000.0f)) {
        snprintf(err_buf, err_len, "water_level_cm di luar batas fisis (0..1000 cm): %.1f", p.water_level_cm);
        return false;
    }
    if (p.ammonia_ppm != -1.0f && (p.ammonia_ppm < 0.0f || p.ammonia_ppm > 5000.0f)) {
        snprintf(err_buf, err_len, "ammonia_ppm di luar batas fisis (0..5000): %.1f", p.ammonia_ppm);
        return false;
    }
    if (p.h2s_ppm != -1.0f && (p.h2s_ppm < 0.0f || p.h2s_ppm > 5000.0f)) {
        snprintf(err_buf, err_len, "h2s_ppm di luar batas fisis (0..5000): %.1f", p.h2s_ppm);
        return false;
    }
    if (p.battery_voltage != -1.0f && (p.battery_voltage < 0.0f || p.battery_voltage > 30.0f)) {
        snprintf(err_buf, err_len, "battery_voltage di luar batas fisis (0..30V): %.1f", p.battery_voltage);
        return false;
    }

    return true;
}

// ==========================================
// 4. ISR INTERRUPT DIO0 (HARDWARE RX_DONE)
// ==========================================
void IRAM_ATTR isr_lora_rx() {
    if (TaskLoRaRxHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(TaskLoRaRxHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ==========================================
// 5. TASK: PENERIMA LORA RX ASINKRON (Core 1, Prio 3) — SOLE RADIO OWNER
// ==========================================
void vTaskLoRaRx(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        // Blokir sampai ada sinyal interupsi hardware DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (!g_radio_available) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // 1. Verifikasi panjang paket aktual dari register radio SX1278
        size_t packetLen = radio.getPacketLength();
        if (packetLen != sizeof(TelemetryPayload)) {
            Serial.printf("[LORA RX REJECTED] Ukuran paket tidak sesuai kontrak: %u bytes (diharapkan %u bytes). CRC: RF Hardware Check.\n",
                          (unsigned int)packetLen, (unsigned int)sizeof(TelemetryPayload));
            radio.startReceive();
            continue;
        }

        TelemetryPayload rxData;
        memset(&rxData, 0, sizeof(TelemetryPayload));

        int state = radio.readData((uint8_t*)&rxData, sizeof(TelemetryPayload));
        if (state != RADIOLIB_ERR_NONE) {
            Serial.printf("[LORA RX ERROR] Gagal membaca data radio (Kode: %d)\n", state);
            radio.startReceive();
            continue;
        }

        // 2. Validasi Skema & Flag
        if (rxData.schema_version != 1) {
            Serial.printf("[LORA RX REJECTED] Versi skema tidak dikenal: %u\n", rxData.schema_version);
            radio.startReceive();
            continue;
        }

        if (rxData.sos_triggered > 1) {
            Serial.printf("[LORA RX REJECTED] Status SOS tidak sah: %u\n", rxData.sos_triggered);
            radio.startReceive();
            continue;
        }

        // 3. Validasi & Sanitasi Identitas Node (node_code)
        char safe_node[9] = {0};
        memcpy(safe_node, rxData.node_code, 8);
        safe_node[8] = '\0';

        if (safe_node[0] == '\0') {
            Serial.println(F("[LORA RX REJECTED] node_code kosong!"));
            radio.startReceive();
            continue;
        }

        bool nodeValid = true;
        for (size_t i = 0; i < strlen(safe_node); i++) {
            if (!isalnum((unsigned char)safe_node[i]) && safe_node[i] != '_' && safe_node[i] != '-') {
                nodeValid = false;
                break;
            }
        }
        if (!nodeValid) {
            Serial.printf("[LORA RX REJECTED] node_code mengandung karakter non-printable!\n");
            radio.startReceive();
            continue;
        }

        // 4. Validasi Numerik Float
        char floatErr[80] = {0};
        if (!validateTelemetryFloats(rxData, floatErr, sizeof(floatErr))) {
            Serial.printf("[LORA RX REJECTED] Kontrak data float dilanggar: %s\n", floatErr);
            radio.startReceive();
            continue;
        }

        // 5. Pencatatan Waktu: Simpan Waktu Penerimaan RTC Secara Terpisah
        GatewayTelemetryRecord record;
        record.payload = rxData; // Mempertahankan uptime_seconds asli dari Node WC tanpa overwrite
        
        if (g_rtc_available) {
            record.gateway_timestamp = rtc.now().unixtime();
        } else {
            record.gateway_timestamp = 0; // Sentinel 0: Waktu RTC offline / belum tersinkronisasi
        }

        Serial.printf("[LORA RX] Paket Sah: Node=%.8s | Seq=#%u | Node Uptime=%u s | Gateway Epoch=%u | Air=%.1f cm | Batt=%.2f V | RSSI=%.1f dBm | SNR=%.1f dB\n",
                      safe_node, record.payload.sequence_no, record.payload.uptime_seconds, record.gateway_timestamp,
                      record.payload.water_level_cm, record.payload.battery_voltage,
                      radio.getRSSI(), radio.getSNR());

        // 6. Masukkan ke antrean telemetri RAM. Jika RAM penuh, simpan ke LittleFS
        if (xQueueSend(xQueueTelemetry, &record, 0) != pdPASS) {
            Serial.println(F("[LORA RX] Antrean RAM penuh, menyimpan ke LittleFS."));
            pushToBuffer(record);
        }

        // Catatan Operasional: Dalam fase Uplink-Only, Gateway TIDAK memancarkan downlink.
        // Radio segera dikembalikan ke mode siaga penerimaan kontinu.
        radio.startReceive();
    }
}

// ==========================================
// 6. TASK: MQTT DISPATCH & STORE-AND-FORWARD (Core 0, Prio 2)
// ==========================================
void vTaskMqttTx(void *pvParameters) {
    (void)pvParameters;
    uint8_t ram_serviced_count = 0;

    for (;;) {
        GatewayTelemetryRecord record;
        bool hasData = false;
        bool isFromFlash = false;
        CommitToken flashToken = {0, 0, false};

        // Pengecekan status koneksi MQTT (Thread-safe via mqttMutex)
        bool isMqttOnline = false;
        if (mqttMutex != NULL) {
            xSemaphoreTake(mqttMutex, portMAX_DELAY);
            isMqttOnline = mqtt.connected();
            xSemaphoreGive(mqttMutex);
        }

        // Fair Scheduling: Jika ada backlog di flash dan MQTT online, jangan biarkan
        // paket RAM memonopoli jaringan secara terus menerus (Starvation Prevention).
        // Setiap 2 paket RAM, beri giliran 1 paket flash.
        if (isMqttOnline && meta.count > 0 && ram_serviced_count >= 2) {
            if (peekBuffer(&record, &flashToken)) {
                hasData = true;
                isFromFlash = true;
                ram_serviced_count = 0;
            }
        }

        // Jika belum mendapatkan data dari flash, coba ambil dari antrean RAM
        if (!hasData) {
            if (xQueueReceive(xQueueTelemetry, &record, pdMS_TO_TICKS(50)) == pdTRUE) {
                hasData = true;
                isFromFlash = false;
                ram_serviced_count++;
            }
            // Jika RAM kosong dan flash memiliki backlog saat MQTT online
            else if (isMqttOnline && meta.count > 0) {
                if (peekBuffer(&record, &flashToken)) {
                    hasData = true;
                    isFromFlash = true;
                    ram_serviced_count = 0;
                }
            }
        }

        if (hasData) {
            bool published = false;

            if (isMqttOnline && mqttMutex != NULL) {
                xSemaphoreTake(mqttMutex, portMAX_DELAY);
                if (mqtt.connected()) {
                    StaticJsonDocument<384> doc;
                    doc["schema_version"]  = record.payload.schema_version;

                    char safe_node[9] = {0};
                    memcpy(safe_node, record.payload.node_code, 8);
                    safe_node[8] = '\0';
                    doc["node_code"]       = safe_node;

                    doc["sequence_no"]     = record.payload.sequence_no;
                    // Sesuai kontrak backend Go: field "timestamp" adalah Unix Epoch saat diterima
                    doc["timestamp"]       = record.gateway_timestamp;
                    // Uptime durasi operasional asli dari Node WC tetap dicantumkan
                    doc["uptime_seconds"]  = record.payload.uptime_seconds;
                    doc["water_level_cm"]  = record.payload.water_level_cm;
                    doc["ammonia_ppm"]     = record.payload.ammonia_ppm;
                    doc["h2s_ppm"]         = record.payload.h2s_ppm;
                    doc["battery_voltage"] = record.payload.battery_voltage;
                    doc["sos_triggered"]   = record.payload.sos_triggered;

                    char jsonBuffer[384];
                    size_t jsonLen = serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

                    // Eksekusi publish MQTT
                    if (mqtt.publish(MQTT_TOPIC_TELEMETRY, (const uint8_t*)jsonBuffer, (unsigned int)jsonLen, true)) {
                        Serial.printf("[MQTT TX] Sukses Publish ke %s (%s): %s\n",
                                      MQTT_TOPIC_TELEMETRY, isFromFlash ? "FLASH" : "RAM", jsonBuffer);
                        published = true;
                    } else {
                        Serial.println(F("[MQTT TX ERROR] Publish gagal ke broker MQTT!"));
                    }
                }
                xSemaphoreGive(mqttMutex);
            }

            // Two-Phase Commit Handling:
            if (isFromFlash) {
                if (published) {
                    // Tahap 2: Commit penghapusan dari LittleFS hanya setelah publish berhasil
                    commitBufferDeletion(flashToken);
                } else {
                    // Publish gagal: data tetap berada di LittleFS head. Dilarang pushToBuffer ulang!
                    Serial.println(F("[STORE-AND-FORWARD] Publish data flash gagal; record dipertahankan di buffer."));
                    vTaskDelay(pdMS_TO_TICKS(500)); // Beri jeda agar tidak retry beruntun tanpa henti
                }
            } else {
                // Data dari RAM yang gagal dipublish disimpan ke LittleFS
                if (!published) {
                    Serial.println(F("[STORE-AND-FORWARD] Publish data RAM gagal/offline; dialihkan ke LittleFS."));
                    pushToBuffer(record);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==========================================
// 7. TASK: SUPERVISOR WIFI & MQTT (Core 0, Prio 1)
// ==========================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (topic == NULL) return;

    // 1. Penanganan Topik Sinkronisasi Waktu Server Backend
    if (strcmp(topic, MQTT_TOPIC_TIMESYNC) == 0) {
        if (length == 0 || length > 32) {
            Serial.println(F("[TIME_SYNC ERROR] Panjang payload time_sync tidak sah."));
            return;
        }

        char time_str[33] = {0};
        memcpy(time_str, payload, length);
        time_str[length] = '\0';

        bool allDigits = true;
        for (size_t i = 0; i < length; i++) {
            if (!isdigit((unsigned char)time_str[i])) {
                allDigits = false;
                break;
            }
        }
        if (!allDigits) {
            Serial.printf("[TIME_SYNC ERROR] Format payload bukan angka bulat sah: '%s'\n", time_str);
            return;
        }

        unsigned long server_epoch = strtoul(time_str, NULL, 10);
        // Validasi epoch waktu masuk akal (antara 1 Nov 2023 [1700000000] hingga tahun 2050 [2500000000])
        if (server_epoch >= 1700000000UL && server_epoch < 2500000000UL) {
            if (g_rtc_available || rtc.begin()) {
                rtc.adjust(DateTime((uint32_t)server_epoch));
                g_rtc_available = true;
                Serial.printf("[RTC SYNC] RTC DS3231 berhasil disinkronkan ke Epoch Server: %lu\n", server_epoch);
            } else {
                Serial.println(F("[RTC SYNC ERROR] RTC DS3231 offline / tidak merespons di I2C!"));
            }
        } else {
            Serial.printf("[TIME_SYNC ERROR] Nilai epoch di luar rentang sah (1.7B - 2.5B): %lu\n", server_epoch);
        }
        return;
    }

    // 2. Validasi Topik Komando Downlink
    if (strncmp(topic, "esos/", 5) != 0 || strstr(topic, "/command") == NULL) {
        Serial.println(F("[MQTT RX] Pesan diabaikan: Topik tidak dikenal atau di luar namespace esos!"));
        return;
    }

    // 3. Validasi Panjang Payload Komando
    if (length == 0 || length > 256) {
        Serial.println(F("[MQTT RX ERROR] Panjang payload tidak valid (0 atau > 256 byte)."));
        return;
    }

    // 4. Deserialisasi JSON Komando
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.printf("[MQTT RX ERROR] Format JSON rusak: %s\n", error.c_str());
        return;
    }

    // 5. Wajibkan Seluruh Field (DILARANG MENGGUNAKAN NILAI DEFAULT)
    if (!doc.containsKey("node_code") || !doc.containsKey("command_id") || !doc.containsKey("parameter")) {
        Serial.println(F("[MQTT RX ERROR] Field wajib hilang! Wajib menyertakan: node_code, command_id, parameter."));
        return;
    }

    // 6. Validasi Tipe Data JSON
    if (!doc["node_code"].is<const char*>() || 
        !doc["command_id"].is<unsigned int>() || 
        !doc["parameter"].is<unsigned int>()) {
        Serial.println(F("[MQTT RX ERROR] Tipe data JSON salah (wajib string node_code, integer command_id & parameter)."));
        return;
    }

    const char* node_str = doc["node_code"].as<const char*>();
    size_t node_len = strlen(node_str);
    if (node_len == 0) {
        Serial.println(F("[MQTT RX ERROR] Field node_code kosong!"));
        return;
    }
    if (node_len > 7) {
        Serial.printf("[MQTT RX ERROR] node_code '%s' terlalu panjang (> 7 karakter)! Ditolak tanpa truncating.\n", node_str);
        return;
    }
    for (size_t i = 0; i < node_len; i++) {
        if (!isalnum((unsigned char)node_str[i]) && node_str[i] != '_' && node_str[i] != '-') {
            Serial.printf("[MQTT RX ERROR] node_code '%s' mengandung karakter ilegal!\n", node_str);
            return;
        }
    }

    unsigned int raw_cmd = doc["command_id"].as<unsigned int>();
    if (raw_cmd < 1 || raw_cmd > 4) { // 1: LOCK, 2: UNLOCK, 3: FLUSH, 4: PING
        Serial.printf("[MQTT RX ERROR] command_id %u tidak dikenal (rentang sah: 1..4)!\n", raw_cmd);
        return;
    }

    unsigned int raw_param = doc["parameter"].as<unsigned int>();
    if (raw_param > 255) {
        Serial.printf("[MQTT RX ERROR] parameter %u overflow (> 255)!\n", raw_param);
        return;
    }
    if ((raw_cmd == 1 || raw_cmd == 2) && raw_param > 180) {
        Serial.printf("[MQTT RX ERROR] Sudut servo %u di luar batas (0..180)!\n", raw_param);
        return;
    }

    // 7. Pemisahan Validasi Format dari Status Fitur Downlink
#if !ENABLE_GATEWAY_DOWNLINK
    // Status operasional default: Uplink-Only. Downlink dinonaktifkan secara sadar.
    Serial.printf("[MQTT RX STATUS: DOWNLINK_UNAVAILABLE] Komando valid untuk '%.8s' (CMD=%u, PARAM=%u) DITOLAK.\n",
                  node_str, raw_cmd, raw_param);
    Serial.println(F("   -> Node WC saat ini berjalan dalam mode Uplink-Only (tidak membuka RX window)."));
#else
    // Mode pengembangan downlink (jika diaktifkan via feature flag)
    Serial.printf("[MQTT RX DEV] Komando valid diterima untuk '%.8s' (CMD=%u, PARAM=%u)\n",
                  node_str, raw_cmd, raw_param);
#endif
}

void vTaskWiFiSupervisor(void *pvParameters) {
    (void)pvParameters;
    uint32_t backoff = 3000;
    static bool s_wifi_connected_logged = false;

    // Persiapkan Alamat IP Statis Target (192.168.101.11 / 24)
    IPAddress local_IP(STATIC_IP_LOCAL);
    IPAddress gateway_IP(STATIC_IP_GATEWAY);
    IPAddress subnet_mask(STATIC_IP_SUBNET);
    IPAddress dns_server(STATIC_IP_DNS);

    for (;;) {
        // Monitor Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            s_wifi_connected_logged = false;
            WiFi.disconnect();

            // Terapkan IP Statis sebelum WiFi.begin()
            if (!WiFi.config(local_IP, gateway_IP, subnet_mask, dns_server)) {
                Serial.println(F("[WIFI ERROR] Penerapan IP Statis (WiFi.config) GAGAL! Periksa parameter subnet."));
            } else {
                Serial.println(F("[WIFI] Konfigurasi IP Statis (192.168.101.11) diterapkan."));
            }

            WiFi.begin(WIFI_SSID, WIFI_PASS);
            Serial.printf("[WIFI] Menyambungkan ke SSID '%s'...\n", WIFI_SSID);
            vTaskDelay(pdMS_TO_TICKS(5000));
        } else if (!s_wifi_connected_logged) {
            Serial.printf("[WIFI OK] Terhubung ke SSID '%s'! IP: %s | GW: %s | Netmask: %s | DNS: %s\n",
                          WIFI_SSID,
                          WiFi.localIP().toString().c_str(),
                          WiFi.gatewayIP().toString().c_str(),
                          WiFi.subnetMask().toString().c_str(),
                          WiFi.dnsIP().toString().c_str());
            s_wifi_connected_logged = true;
        }

        // Monitor MQTT jika Wi-Fi sudah tersambung
        bool isConnected = false;
        if (mqttMutex != NULL) {
            xSemaphoreTake(mqttMutex, portMAX_DELAY);
            isConnected = mqtt.connected();
            xSemaphoreGive(mqttMutex);
        }

        if (WiFi.status() == WL_CONNECTED && !isConnected) {
            if (mqttMutex != NULL) {
                xSemaphoreTake(mqttMutex, portMAX_DELAY);
                mqtt.setServer(MQTT_SERVER, MQTT_PORT);
                mqtt.setCallback(mqttCallback);

                String macClean = WiFi.macAddress();
                macClean.replace(":", "");
                String clientId = "esos_gateway_" + macClean;
                Serial.printf("[MQTT] Menghubungi Broker %s:%d dengan ClientID: %s...\n",
                              MQTT_SERVER, MQTT_PORT, clientId.c_str());

                if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
                    Serial.println(F("[MQTT OK] Terhubung ke Broker Mosquitto!"));
                    mqtt.publish(MQTT_TOPIC_STATUS, "ONLINE", true);
                    mqtt.subscribe(MQTT_TOPIC_COMMAND, 1);
                    mqtt.subscribe(MQTT_TOPIC_TIMESYNC, 1);
                    backoff = 3000;
                } else {
                    Serial.printf("[MQTT] Gagal terhubung (rc=%d). Backoff %u ms\n", mqtt.state(), backoff);
                    if (backoff < 30000) backoff *= 2;
                }
                xSemaphoreGive(mqttMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(backoff));
        }

        if (mqttMutex != NULL) {
            xSemaphoreTake(mqttMutex, portMAX_DELAY);
            if (mqtt.connected()) {
                mqtt.loop();
            }
            xSemaphoreGive(mqttMutex);
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

    Serial.println();
    Serial.println(F("================================================================="));
    Serial.println(F("  SMART-SANITATION eSOS - GATEWAY ROUTER FIRMWARE (ESP32 DevKit) "));
    Serial.println(F("================================================================="));

#if HAS_CONFIG_LOCAL
    Serial.println(F("[CONFIG] Memuat konfigurasi jaringan dari config_local.h (kredensial terlindungi)."));
#else
    Serial.println(F("[CONFIG NOTICE] config_local.h tidak ditemukan. Menggunakan konfigurasi default internal."));
#endif

    // 1. Inisialisasi Mutex & Queues FreeRTOS
    fsMutex = xSemaphoreCreateMutex();
    mqttMutex = xSemaphoreCreateMutex();
    xQueueTelemetry = xQueueCreate(20, sizeof(GatewayTelemetryRecord));

    if (fsMutex == NULL || mqttMutex == NULL || xQueueTelemetry == NULL) {
        Serial.println(F("[FATAL] Gagal membuat kernel primitives FreeRTOS! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 2. Inisialisasi Bus I2C RTC DS3231 (SDA:4, SCL:22)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (!rtc.begin()) {
        Serial.println(F("[RTC PERINGATAN] Modul RTC DS3231 tidak ditemukan pada I2C!"));
        g_rtc_available = false;
    } else if (rtc.lostPower()) {
        Serial.println(F("[RTC PERINGATAN] DS3231 lost power / osilator berhenti, waktu belum sinkron!"));
        g_rtc_available = false;
    } else {
        Serial.println(F("[RTC OK] Modul RTC DS3231 aktif dan waktu valid."));
        g_rtc_available = true;
    }

    // 3. Konfigurasi buffer PubSubClient (Wajib 512B untuk JSON 384B)
    mqtt.setBufferSize(512);

    // 4. Inisialisasi LittleFS (Store-and-Forward Flash Disk)
    // Gunakan formatOnFail = false agar kegagalan mount tidak menghapus paksa backlog flash
    if (!LittleFS.begin(false)) {
        Serial.println(F("[LITTLEFS ERROR] Mount LittleFS gagal! Data flash dipertahankan tanpa autoformat."));
        Serial.println(F("[LITTLEFS NOTICE] Berjalan dalam mode degraded (RAM buffer only)."));
        g_fs_available = false;
    } else {
        g_fs_available = true;
        loadMeta();
        Serial.printf("[OK] LittleFS aktif. Antrean pesan tersimpan: %u (Dropped: %u)\n", 
                      meta.count, meta.dropped_count);
    }

    // 5. Inisialisasi Perangkat Keras SPI Bus & Reset SX1278
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100);

    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);

    // 6. Inisialisasi Radio SX1278
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        radio.setCRC(true);
        g_radio_available = true;
        Serial.println(F("[OK] Radio SX1278 aktif di 433.175 MHz."));
    } else {
        g_radio_available = false;
        Serial.printf("[RADIO ERROR] Inisialisasi Radio SX1278 gagal (Kode: %d). Memeriksa jumper SPI...\n", state);
    }

    // 7. Peluncuran Task FreeRTOS ke Dual Core ESP32
    // Core 0: Wi-Fi/MQTT Supervisor dan MQTT Dispatcher
    // Core 1: Sole Radio Owner (TaskLoRaRx)
    BaseType_t rWifi = xTaskCreatePinnedToCore(vTaskWiFiSupervisor, "TaskWiFi", 4096, NULL, 1, NULL, 0);
    BaseType_t rMqtt = xTaskCreatePinnedToCore(vTaskMqttTx,         "TaskMqtt", 4096, NULL, 2, NULL, 0);
    BaseType_t rRx   = xTaskCreatePinnedToCore(vTaskLoRaRx,         "TaskLoRaRx", 4096, NULL, 3, &TaskLoRaRxHandle, 1);

    if (rWifi != pdPASS || rMqtt != pdPASS || rRx != pdPASS) {
        Serial.println(F("[FATAL] Gagal meluncurkan Task FreeRTOS Gateway! Halting."));
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 8. URUTAN INISIALISASI KRITIS:
    // Pasang callback ISR DIO0 dan aktifkan RX Continuous HANYA SETELAH TaskLoRaRxHandle valid!
    // Mencegah hilangnya event interrupt awal akibat handle bernilai NULL.
    if (g_radio_available && TaskLoRaRxHandle != NULL) {
        radio.setDio0Action(isr_lora_rx, RISING);
        radio.startReceive();
        Serial.println(F("[OK] ISR DIO0 terhubung ke TaskLoRaRxHandle. Radio siaga dalam mode RX Continuous."));
    }

    Serial.println(F("=================================================================\n"));
}

void loop() {
    vTaskDelete(NULL);
}
