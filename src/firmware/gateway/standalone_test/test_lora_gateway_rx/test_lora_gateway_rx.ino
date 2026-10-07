/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI PENERIMA MANDIRI (RECEIVER / GATEWAY): ESP32 DEVKIT V1 + Ai-Thinker Ra-02
 * ======================================================================================
 *
 * 1. IDENTIFIKASI BOARD & HARDWARE:
 *    - Board Target   : DOIT ESP32 DevKit V1 (30-pin, ESP32-WROOM-32)
 *    - Modul Radio    : Ai-Thinker Ra-02 (Semtech SX1278)
 *    - Pin Interupsi  : DIO0 terhubung ke GPIO 2 (Strapping Pin & LED Onboard Biru)
 *      * PERINGATAN FISIK: Pastikan DIO0 tidak menahan tegangan HIGH saat ESP32 boot.
 *        Jika terjadi kegagalan boot/flash, pindahkan pin DIO0 ke GPIO 4.
 *      * Catu Daya: VCC Ra-02 WAJIB 3.3V (Dilarang ke 5V/VIN).
 *
 * 2. KEPATUHAN REGULASI SPEKTRUM RADIO INDONESIA:
 *    - Rujukan Regulasi: Peraturan Menteri Komunikasi dan Digital (Permenkomdigi) No. 2 Tahun 2025
 *      (Pita Frekuensi Radio untuk Keperluan Perangkat Jarak Dekat / LPWAN Non-Lisensi).
 *    - Alokasi Frekuensi: 433,050 MHz – 434,790 MHz dengan Bandwidth Kanal Maksimum 125 kHz.
 *    - Frekuensi Uji   : 433.175 MHz (Pusat kanal rentang nominal 433,1125 – 433,2375 MHz).
 *    - Batas Emisi     : Maksimum 12,15 dBm EIRP (setara 10 dBm ERP / 10 mW).
 *    - Catatan EIRP    : EIRP = P_conducted - L_kabel + G_antena. Frekuensi tengah saja tidak
 *      menjamin kepatuhan mutlak tanpa memperhitungkan penguatan antena dan daya pemancar.
 *
 * 3. ARSITEKTUR FREERTOS & REALITAS KEANDALAN:
 *    - Core 0 = PRO_CPU (Menjalankan Task Dispatcher / Pemrosesan Log)
 *    - Core 1 = APP_CPU (Menjalankan Task Radio RX Listener)
 *    - SX1278 memiliki FIFO internal 256 byte di silikon chip. Antrean FreeRTOS berada di RAM.
 *    - PENTING: Penggunaan dua task dan antrean TIDAK menjamin paket tidak pernah hilang.
 *      Paket tetap dapat hilang akibat tabrakan gelombang di udara (LoRa ALOHA uncoordinated),
 *      redaman struktur beton/path loss, atau jika register FIFO tertimpa sebelum sempat dibaca.
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <ctype.h>
#include <math.h>

// ===================================================================
// 1. PINOUT PERANGKAT KERAS (SESUAI WIRING JUMPER FISIK AKTIF)
// ===================================================================
#define PIN_LORA_SCK      21    // Cokelat (D21 ESP32)
#define PIN_LORA_MISO     19    // Merah   (D19 ESP32)
#define PIN_LORA_MOSI     18    // Oranye  (D18 ESP32)
#define PIN_LORA_NSS      5     // Kuning  (D5  ESP32)
#define PIN_LORA_DIO0     2     // Ungu    (D2  ESP32) - Interupsi RX_DONE
#define PIN_LORA_RESET    15    // Biru    (D15 ESP32) - Reset Hardware SX1278

// ===================================================================
// 2. PARAMETER MODULASI RF (IDENTIK DENGAN TRANSMITTER STANDALONE)
// ===================================================================
#define LORA_FREQ         433.175 // Frekuensi tengah kanal (MHz)
#define LORA_BW           125.0   // Bandwidth kanal (kHz)
#define LORA_SF           9       // Spreading Factor
#define LORA_CR           7       // Coding Rate 4/7 (CR = 3)
#define LORA_SYNC_WORD    0x12    // Sync Word Jaringan Privat eSOS (SX127X)

// ===================================================================
// 3. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK (34 Bytes)
// ===================================================================
// Format Biner: Little-Endian (Arsitektur Xtensa 32-bit LX6 ESP32).
// Ukuran Total: Tepat 34 Bytes tanpa padding compiler.
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Versi skema biner (Wajib 1)
    char node_code[8];      // Offset  1 | 8 bytes : Identitas node (String null-terminated)
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket (Little-Endian)
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu operasional sejak boot (Detik)
    float water_level_cm;   // Offset 17 | 4 bytes : Level air simulasi (IEEE-754)
    float ammonia_ppm;      // Offset 21 | 4 bytes : Gas amonia simulasi (IEEE-754)
    float h2s_ppm;          // Offset 25 | 4 bytes : Gas H2S simulasi (IEEE-754)
    float battery_voltage;  // Offset 29 | 4 bytes : Tegangan baterai simulasi (IEEE-754)
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : Flag status darurat (0 = Normal, 1 = SOS)
};
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// Wrapper data untuk antrean gateway (Payload + Metrik RF Aktual)
struct GatewayPacketWrapper {
    TelemetryPayload payload;
    float rssi;
    float snr;
    size_t raw_length;
};

// Struktur Pelacak Multi-Node (Ditempatkan di awal agar dikenal oleh Arduino prototype preprocessor)
#define MAX_TRACKED_NODES 8

struct NodeTracker {
    char node_code[8];
    uint32_t last_seq;
    uint32_t total_received;
    uint32_t observed_gaps;
    uint32_t duplicate_count;
    uint32_t restarts_or_wraparounds;
    bool active;
};

// ===================================================================
// 4. INSTANSIASI HARDWARE & OBJEK KERNEL FREERTOS
// ===================================================================
// Argumen ke-4 Module(cs, irq, rst, gpio) adalah RADIOLIB_NC (bukan MISO), oper objek SPI secara eksplisit
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);

// Handle Kernel FreeRTOS
static QueueHandle_t xGatewayQueue = NULL;
static TaskHandle_t xLoRaTaskHandle = NULL;

// Statistik Diagnostik Radio & Antrean
static volatile uint32_t statCrcErrorCount = 0;
static volatile uint32_t statLengthMismatchCount = 0;
static volatile uint32_t statValidationFailCount = 0;
static volatile uint32_t statDroppedQueueFullCount = 0;
static volatile uint32_t statRearmFailureCount = 0;

// ===================================================================
// 5. INTERRUPT SERVICE ROUTINE (ISR) - HARDWARE DIO0
// ===================================================================
// ISR harus seringan mungkin: hanya memberi notifikasi langsung ke Task tanpa SPI / Serial
void IRAM_ATTR isrDio0RxDone() {
    if (xLoRaTaskHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(xLoRaTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// ===================================================================
// 6. FUNGSI VALIDASI PAYLOAD TANPA MENGUBAH DATA ASLI
// ===================================================================
static bool validateTelemetryPayload(const TelemetryPayload *p) {
    if (p == NULL) return false;

    // 1. Validasi Schema Version
    if (p->schema_version != 1) {
        return false;
    }

    // 2. Validasi node_code TANPA MEMODIFIKASI ISI BUFFER:
    // Wajib memiliki karakter null terminator '\0' dalam batas 8 byte
    bool hasTerminator = false;
    size_t nameLen = 0;
    for (size_t i = 0; i < sizeof(p->node_code); i++) {
        if (p->node_code[i] == '\0') {
            hasTerminator = true;
            nameLen = i;
            break;
        }
    }
    if (!hasTerminator || nameLen == 0) {
        return false;
    }

    // Karakter sebelum terminator harus berupa ASCII alfanumerik yang valid, tanda '-', atau '_'
    for (size_t i = 0; i < nameLen; i++) {
        unsigned char c = (unsigned char)p->node_code[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }

    // 3. Validasi Flag SOS (Hanya boleh 0 atau 1)
    if (p->sos_triggered > 1) {
        return false;
    }

    // 4. Validasi Nilai Numerik Float (Mencegah NaN atau Infinity yang merusak log)
    if (isnan(p->water_level_cm) || isinf(p->water_level_cm) ||
        isnan(p->ammonia_ppm)    || isinf(p->ammonia_ppm)    ||
        isnan(p->h2s_ppm)        || isinf(p->h2s_ppm)        ||
        isnan(p->battery_voltage)|| isinf(p->battery_voltage)) {
        return false;
    }

    return true;
}

// ===================================================================
// 7. TASK 1: LORA RX LISTENER (CORE 1 / APP_CPU - PRIORITAS 3)
// ===================================================================
// Bertugas membaca paket dari SX1278 secepat mungkin dan segera me-rearm mode RX
void vTaskLoRaRxListener(void *pvParameters) {
    for (;;) {
        // Blokir task sampai ada notifikasi direct-to-task dari ISR DIO0
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 1. Periksa panjang paket aktual dari register SX1278 sebelum membaca payload
        size_t packetLen = radio.getPacketLength();

        if (packetLen != sizeof(TelemetryPayload)) {
            statLengthMismatchCount++;

            // Kuras buffer FIFO SX1278 secara aman dengan buffer kecil tetap (32 bytes).
            // Sesuai implementasi RadioLib SX127x::readData, jika requested len < packet length,
            // RadioLib akan membaca 32 bytes lalu memanggil clearFIFO(dumpLen) untuk menguras sisa FIFO!
            uint8_t drainBuf[32];
            int drainState = radio.readData(drainBuf, sizeof(drainBuf));
            (void)drainState; // Status drain dicatat tanpa menghentikan sistem

        } else {
            // Panjang tepat 34 bytes, baca data payload biner dari FIFO
            GatewayPacketWrapper incoming;
            memset(&incoming, 0, sizeof(GatewayPacketWrapper));
            incoming.raw_length = packetLen;

            int readState = radio.readData((uint8_t*)&incoming.payload, sizeof(TelemetryPayload));

            if (readState == RADIOLIB_ERR_NONE) {
                // Verifikasi validitas payload biner tanpa mengubah isinya
                if (validateTelemetryPayload(&incoming.payload)) {
                    incoming.rssi = radio.getRSSI();
                    incoming.snr = radio.getSNR();

                    // Kirim ke Antrean FreeRTOS secara non-blocking (timeout = 0)
                    // agar tidak menunda pengaktifan kembali mode RX
                    if (xQueueSend(xGatewayQueue, &incoming, 0) != pdTRUE) {
                        statDroppedQueueFullCount++;
                    }
                } else {
                    statValidationFailCount++;
                }

            } else if (readState == RADIOLIB_ERR_CRC_MISMATCH) {
                // RadioLib mendeteksi flag PayloadCrcError pada register IRQ SX1278
                statCrcErrorCount++;
            } else {
                // Kesalahan pembacaan SPI atau status radio lainnya
                statValidationFailCount++;
            }
        }

        // 2. SEGERA re-arm mode RX asinkron agar window penerimaan radio kembali aktif
        int rearmState = radio.startReceive();
        if (rearmState != RADIOLIB_ERR_NONE) {
            // Prosedur pemulihan eksplisit: set ke standby terlebih dahulu, lalu coba startReceive kembali
            radio.standby();
            rearmState = radio.startReceive();
            if (rearmState != RADIOLIB_ERR_NONE) {
                statRearmFailureCount++;
                // Beri jeda singkat agar core tidak busy-spin jika hardware mengalami gangguan listrik
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }
}

// ===================================================================
// 8. PELACAK NOMOR URUT PER-NODE (MULTI-NODE TRACKER)
// ===================================================================
static NodeTracker trackedNodes[MAX_TRACKED_NODES];
static uint32_t untrackedNodesCount = 0;

static NodeTracker* getOrRegisterNode(const char *code) {
    // 1. Cari apakah node sudah terdaftar
    for (int i = 0; i < MAX_TRACKED_NODES; i++) {
        if (trackedNodes[i].active && strncmp(trackedNodes[i].node_code, code, 7) == 0) {
            return &trackedNodes[i];
        }
    }
    // 2. Daftarkan di slot kosong pertama
    for (int i = 0; i < MAX_TRACKED_NODES; i++) {
        if (!trackedNodes[i].active) {
            strncpy(trackedNodes[i].node_code, code, 7);
            trackedNodes[i].node_code[7] = '\0';
            trackedNodes[i].last_seq = 0;
            trackedNodes[i].total_received = 0;
            trackedNodes[i].observed_gaps = 0;
            trackedNodes[i].duplicate_count = 0;
            trackedNodes[i].restarts_or_wraparounds = 0;
            trackedNodes[i].active = true;
            return &trackedNodes[i];
        }
    }
    // Tabel penuh: kembalikan NULL
    return NULL;
}

// ===================================================================
// 9. TASK 2: DISPATCHER / SERIAL LOGGER (CORE 0 / PRO_CPU - PRIORITAS 1)
// ===================================================================
// Bertugas memproses data dari antrean, menganalisis nomor urut, dan logging ke Serial
void vTaskGatewayDispatch(void *pvParameters) {
    GatewayPacketWrapper item;

    for (;;) {
        // Ambil paket dari antrean buffer (Task tidur jika antrean kosong)
        if (xQueueReceive(xGatewayQueue, &item, portMAX_DELAY) == pdTRUE) {
            
            // Siapkan string aman untuk pencetakan (dijamin null-terminated 9 byte)
            char safeNodeCode[9];
            memset(safeNodeCode, 0, sizeof(safeNodeCode));
            memcpy(safeNodeCode, item.payload.node_code, sizeof(item.payload.node_code));
            safeNodeCode[8] = '\0';

            NodeTracker *node = getOrRegisterNode(safeNodeCode);
            uint32_t currentGap = 0;
            bool isDuplicate = false;
            bool isRestart = false;

            if (node != NULL) {
                node->total_received++;

                if (node->last_seq == 0) {
                    // Paket pertama yang diamati dari node ini
                    node->last_seq = item.payload.sequence_no;
                } else if (item.payload.sequence_no == node->last_seq) {
                    // Nomor urut sama persis (Duplikat)
                    node->duplicate_count++;
                    isDuplicate = true;
                } else if (item.payload.sequence_no < node->last_seq) {
                    // Nomor urut mundur: kemungkinan node mengalami restart/reboot atau wrap-around uint32
                    node->restarts_or_wraparounds++;
                    isRestart = true;
                    node->last_seq = item.payload.sequence_no;
                } else if (item.payload.sequence_no > node->last_seq + 1) {
                    // Terdeteksi celah nomor urut
                    currentGap = item.payload.sequence_no - (node->last_seq + 1);
                    node->observed_gaps += currentGap;
                    node->last_seq = item.payload.sequence_no;
                } else {
                    // Paket monotonik berurutan normal
                    node->last_seq = item.payload.sequence_no;
                }
            } else {
                untrackedNodesCount++;
            }

            // Dapatkan sisa stack dispatcher untuk diagnostik memori FreeRTOS
            UBaseType_t stackHighWater = uxTaskGetStackHighWaterMark(NULL);

            Serial.println("=================================================================");
            Serial.printf("[GATEWAY DISPATCH] Paket Valid Diterima dari Node: %s\n", safeNodeCode);
            Serial.printf("  Nomor Urut    : #%u\n", item.payload.sequence_no);
            
            if (node != NULL) {
                Serial.printf("  Statistik Node: Total Diterima=%u | Duplikat=%u | Restart/Reset=%u\n",
                              node->total_received, node->duplicate_count, node->restarts_or_wraparounds);
                if (currentGap > 0) {
                    Serial.printf("  PERINGATAN    : Celah nomor urut teramati sebanyak %u paket (Total celah kumulatif: %u)\n",
                                  currentGap, node->observed_gaps);
                    Serial.println("                  (Catatan: Celah dapat disebabkan oleh paket drop di udara,");
                    Serial.println("                   tabrakan frekuensi, redaman gedung, atau antrean penuh).");
                }
            } else {
                Serial.printf("  PERINGATAN    : Tabel pelacak node penuh (Maks %d node)! Paket tidak tercatat di pelacak.\n", MAX_TRACKED_NODES);
            }

            if (isDuplicate) {
                Serial.println("  CATATAN STATUS: Terdeteksi paket DUPLIKAT dengan nomor urut yang sama.");
            }
            if (isRestart) {
                Serial.println("  CATATAN STATUS: Terdeteksi reset nomor urut (diduga node melakukan reboot).");
            }

            Serial.printf("  Kualitas RF   : RSSI = %.1f dBm | SNR = %.2f dB\n", item.rssi, item.snr);
            Serial.printf("  Uptime Node   : %u detik sejak boot\n", item.payload.uptime_seconds);
            Serial.printf("  Telemetri     : Air=%.1f cm | NH3=%.1f ppm | H2S=%.1f ppm | Batt=%.2f V (Simulasi)\n",
                          item.payload.water_level_cm, item.payload.ammonia_ppm, 
                          item.payload.h2s_ppm, item.payload.battery_voltage);
            
            // Kejujuran Status SOS: hanya merujuk pada flag paket ini, bukan klaim kondisi toilet mutlak
            if (item.payload.sos_triggered == 1) {
                Serial.println("  Status SOS    : [DARURAT] Flag SOS aktif pada paket ini.");
            } else {
                Serial.println("  Status SOS    : Flag SOS tidak aktif pada paket ini.");
            }

            Serial.printf("  Diagnostik HW : CRC Error=%u | Len Mismatch=%u | Drop Queue=%u | Rearm Fail=%u | Stack Free=%u words\n",
                          statCrcErrorCount, statLengthMismatchCount, statDroppedQueueFullCount, statRearmFailureCount, (unsigned int)stackHighWater);
            Serial.println("=================================================================\n");
        }
    }
}

// ===================================================================
// 10. SETUP (INISIALISASI SISTEM & URUTAN AMAN BEBAS RACE CONDITION)
// ===================================================================
void setup() {
    Serial.begin(115200);
    delay(2000); // Waktu stabilisasi Serial Monitor laptop

    Serial.println("\n=================================================================");
    Serial.println("  SMART-SANITATION eSOS - GATEWAY RECEIVER TEST (ESP32 DevKit V1)");
    Serial.println("=================================================================");
    Serial.println("Informasi Regulasi & Parameter RF:");
    Serial.println("  - Rujukan Regulasi  : Permenkomdigi No. 2 Tahun 2025 (Pita LPWAN/SRD)");
    Serial.println("  - Rentang Pita      : 433,050 - 434,790 MHz (Bandwidth Maksimum 125 kHz)");
    Serial.printf("  - Frekuensi Tengah  : %.3f MHz (Kanal nominal 433,1125 - 433,2375 MHz)\n", LORA_FREQ);
    Serial.printf("  - Parameter Modulasi: SF%d | BW %.1f kHz | CR 4/%d | SyncWord 0x%02X\n", LORA_SF, LORA_BW, LORA_CR, LORA_SYNC_WORD);
    Serial.printf("  - Kontrak Payload   : %u Bytes (static_assert terverifikasi)\n", (unsigned int)sizeof(TelemetryPayload));
    Serial.println("  - Catatan EIRP      : Kepatuhan emisi nyata bergantung pada EIRP = P_tx - L_kabel + G_antena.");
    Serial.println("=================================================================");

    // Inisialisasi tabel pelacak multi-node
    memset(trackedNodes, 0, sizeof(trackedNodes));

    // LANGKAH 0: Pre-init Pin NSS (CS) ke HIGH agar bus SPI dalam kondisi idle sebelum reset
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);

    // LANGKAH 1: Hard Reset Hardware SX1278 (20ms LOW, lalu 100ms settling time untuk osilator kristal 32MHz Ra-02)
    Serial.printf("1. Melakukan Hardware Reset SX1278 (RST:%d)... ", PIN_LORA_RESET);
    pinMode(PIN_LORA_RESET, OUTPUT);
    digitalWrite(PIN_LORA_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LORA_RESET, HIGH);
    delay(100); // Waktu stabilisasi osilator kristal 32 MHz dan internal regulator SX1278
    Serial.println("[OK]");

    // LANGKAH 2: Inisialisasi Bus SPI Perangkat Keras (SS=-1 agar tidak konflik dengan manual toggle CS RadioLib)
    Serial.printf("2. Inisialisasi Hardware SPI Bus (SCK:%d, MISO:%d, MOSI:%d, SS:Manual)... ",
                  PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, -1);
    Serial.println("[OK]");

    // LANGKAH 3: Inisialisasi Register Chip SX1278 (dengan mekanisme retry otomatis)
    Serial.print("3. Inisialisasi Register Chip Semtech SX1278... ");
    int initState = RADIOLIB_ERR_UNKNOWN;
    for (int attempt = 1; attempt <= 3; attempt++) {
        initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, 10);
        if (initState == RADIOLIB_ERR_NONE) {
            break;
        }
        Serial.printf("[Retry %d/3: %d] ", attempt, initState);
        delay(100);
    }
    if (initState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat: %d\n", initState);
        if (initState == -16) {
            Serial.println("   -> [DIAGNOSIS] Error -16 (SPI_WRITE_FAILED): Chip terdeteksi tetapi register gagal diverifikasi tepat waktu.");
            Serial.println("   -> Coba tekan tombol EN (Reset ESP32) satu kali untuk re-sync power.");
        } else {
            Serial.printf("   -> Periksa kabel jumper SPI (MOSI:%d, MISO:%d, SCK:%d, NSS:%d, RST:%d) dan catu daya 3.3V.\n",
                          PIN_LORA_MOSI, PIN_LORA_MISO, PIN_LORA_SCK, PIN_LORA_NSS, PIN_LORA_RESET);
        }
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // LANGKAH 3: Konfigurasi Hardware CRC secara Eksplisit
    Serial.print("3. Mengonfigurasi Hardware CRC Check (SX1278 RegModemConfig2)... ");
    int crcState = radio.setCRC(true);
    if (crcState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat CRC: %d\n", crcState);
        while (true) delay(1000);
    }
    Serial.println("[OK] CRC Enabled.");

    // LANGKAH 4: Alokasi Queue FreeRTOS & Validasi Status
    Serial.print("4. Mengalokasikan Antrean FreeRTOS Gateway (10 Paket)... ");
    xGatewayQueue = xQueueCreate(10, sizeof(GatewayPacketWrapper));
    if (xGatewayQueue == NULL) {
        Serial.println("[FATAL] Alokasi memori heap untuk Queue gagal! Sistem berhenti.");
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // LANGKAH 5: Pembuatan Task FreeRTOS & Validasi Status
    // Catatan Satuan Stack: Pada ESP-IDF, satuan ukuran stack adalah 'words' (4 bytes per word pada Xtensa 32-bit).
    // 4096 words = 16.384 bytes per task.
    Serial.println("5. Memasang Task FreeRTOS ke Dual Core ESP32:");

    TaskHandle_t xDispatchTaskHandle = NULL;

    // Core 0 (PRO_CPU): Dispatcher Serial & Analisis Data
    BaseType_t dispatchStatus = xTaskCreatePinnedToCore(
        vTaskGatewayDispatch, 
        "TaskDispatch", 
        4096, 
        NULL, 
        1,      // Prioritas 1
        &xDispatchTaskHandle, 
        0       // Core 0 (PRO_CPU)
    );

    // Core 1 (APP_CPU): Radio Listener
    BaseType_t rxTaskStatus = xTaskCreatePinnedToCore(
        vTaskLoRaRxListener, 
        "TaskLoRaRx", 
        4096, 
        NULL, 
        3,      // Prioritas 3
        &xLoRaTaskHandle, 
        1       // Core 1 (APP_CPU)
    );

    if (dispatchStatus != pdPASS || rxTaskStatus != pdPASS || xLoRaTaskHandle == NULL) {
        Serial.println("[FATAL] Gagal membuat salah satu Task FreeRTOS! Program dihentikan (fail-stop).");
        Serial.println("        Catatan: Task yang sempat dibuat akan dibersihkan sebelum berhenti.");
        if (dispatchStatus == pdPASS && xDispatchTaskHandle != NULL) {
            vTaskDelete(xDispatchTaskHandle);
        }
        while (true) delay(1000);
    }
    Serial.println("   -> TaskDispatch (Core 0 / PRO_CPU, Prio 1) : [BERHASIL DIBUAT]");
    Serial.println("   -> TaskLoRaRx   (Core 1 / APP_CPU, Prio 3) : [BERHASIL DIBUAT]");
    Serial.println("      (Pesan ini menandakan alokasi task berhasil, bukan bukti paket sudah diproses)");

    // LANGKAH 6: Mendaftarkan Callback Interupsi Hardware DIO0
    // Callback didaftarkan setelah handle task RX tersedia untuk mencegah ISR menotifikasi handle NULL.
    // (Catatan: Ini mengatasi risiko dereferensi handle NULL saat callback aktif, bukan jaminan seluruh interaksi bebas race).
    Serial.print("6. Mendaftarkan Callback Interupsi DIO0 (GPIO 2)... ");
    radio.setPacketReceivedAction(isrDio0RxDone);
    Serial.println("[OK]");

    // LANGKAH 7: Mengaktifkan Mode Penerimaan Asinkron (startReceive)
    Serial.print("7. Mengaktifkan Radio Mode RX Asinkron (433.175 MHz)... ");
    int rxStartState = radio.startReceive();
    if (rxStartState != RADIOLIB_ERR_NONE) {
        Serial.printf("[GAGAL] Kode Galat startReceive: %d\n", rxStartState);
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    Serial.println("=================================================================");
    Serial.println(">>> MODE RX ASINKRON DIAKTIFKAN DI FREKUENSI 433.175 MHz <<<");
    Serial.println(">>> (Radio siap mendengarkan; keberhasilan penerimaan bergantung pada paket fisik) <<<");
    Serial.println("=================================================================\n");
}

void loop() {
    // Pada Arduino-ESP32, loopTask tetap berjalan di background.
    // Menidurkan task ini mencegah konsumsi CPU tanpa modifikasi heap.
    vTaskDelay(pdMS_TO_TICKS(10000));
}
