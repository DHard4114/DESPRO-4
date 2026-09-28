/*
 * ======================================================================================
 * PROYEK CAPSTONE: SMART-SANITATION eSOS (DESAIN PROYEK 2 - KELOMPOK 4 FTUI)
 * SKRIP UJI MANDIRI PENGIRIM (TRANSMITTER / NODE): ESP32 DEVKIT V1 + Ai-Thinker Ra-02
 * ======================================================================================
 *
 * 1. IDENTIFIKASI BOARD & HARDWARE:
 *    - Target Board   : DOIT ESP32 DevKit V1 (30-pin / ESP32-WROOM-32)
 *    - Modul LoRa     : Ai-Thinker Ra-02 (Semtech SX1278)
 *    - Interface      : Hardware SPI Bus + Dedicated Control Lines
 *
 * 2. TABEL PENGKABELAN FISIK (WIRING HARNESS):
 *    +-------------------+--------------------+---------------------------------------+
 *    | Pin Ra-02 (SX1278)| Pin ESP32 DevKit V1| Fungsi & Catatan Teknis               |
 *    +-------------------+--------------------+---------------------------------------+
 *    | VCC               | 3V3 (3.3V)         | WAJIB 3.3V! (Dilarang ke 5V/VIN)      |
 *    | GND               | GND                | Ground bersama (Common Ground)        |
 *    | NSS (CS)          | GPIO 5             | SPI Chip Select                       |
 *    | MOSI              | GPIO 23            | SPI Master Out Slave In               |
 *    | MISO              | GPIO 19            | SPI Master In Slave Out               |
 *    | SCK               | GPIO 18            | SPI Clock                             |
 *    | RST (RESET)       | GPIO 14            | Reset Hardware SX1278                 |
 *    | DIO0              | GPIO 2             | Interupsi TX_DONE & RX_DONE           |
 *    +-------------------+--------------------+---------------------------------------+
 *    * CATATAN STRAPPING PIN (GPIO 2):
 *      Pada ESP32, GPIO 2 adalah strapping pin (harus LOW saat boot/flashing) dan terhubung
 *      ke onboard LED biru pada DevKit V1. Saat standby/sleep, SX1278 menahan DIO0 LOW
 *      sehingga boot normal. Jika terjadi kegagalan boot/flash saat modul terpasang,
 *      pindahkan DIO0 ke pin alternatif (misal GPIO 4) dan sesuaikan macro PIN_LORA_DIO0.
 *
 * 3. KEPATUHAN REGULASI SPEKTRUM RADIO INDONESIA:
 *    - Regulasi Acuan : Peraturan Menteri Komdigi (Permenkomdigi) No. 2 Tahun 2025 (Pita LPWAN/SRD).
 *    - Alokasi Pita   : 433,050 MHz – 434,790 MHz (Bandwidth kanal maksimum: 125 kHz).
 *    - Frekuensi Uji  : 433.175 MHz (Pusat kanal rentang nominal: 433,1125 – 433,2375 MHz).
 *    - Batas Daya     : Maks. 12,15 dBm EIRP (setara 10 dBm ERP / 10 mW).
 *    - Catatan EIRP   : Kepatuhan emisi nyata bergantung pada EIRP = P_conducted - L_kabel + G_antena.
 *        * Default Lapangan: +10 dBm conducted. Dengan antena 2 dBi dan kabel pendek, EIRP ~12 dBm (<= 12,15 dBm).
 *        * Khusus Lab Berpelindung: +17 dBm (Hanya dengan atenuator RF/dummy load untuk uji terisolasi).
 *
 * 4. ATURAN KESELAMATAN RF & PERINGATAN:
 *    - Antena 433 MHz WAJIB terpasang sebelum modul dinyalakan. Keberhasilan inisialisasi SPI
 *      TIDAK memverifikasi keberadaan antena fisik.
 *    - Status RADIOLIB_ERR_NONE HANYA membuktikan proses modulasi TX di sisi pengirim selesai,
 *      BUKAN jaminan paket diterima di posko atau gelombang RF terpancar jika antena lepas.
 * ======================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// ===================================================================
// 1. DEFINISI PIN PERANGKAT KERAS (ESP32 DevKit V1)
// ===================================================================
#define PIN_LORA_NSS      5     // Chip Select SPI
#define PIN_LORA_DIO0     2     // Interrupt TX/RX Done (Strapping Pin / LED_BUILTIN)
#define PIN_LORA_RESET    14    // Hardware Reset
#define PIN_LORA_MISO     19    // SPI MISO
#define PIN_LORA_MOSI     23    // SPI MOSI
#define PIN_LORA_SCK      18    // SPI SCK

// ===================================================================
// 2. PARAMETER MODULASI RF (PATUH REGULASI KOMINFO INDONESIA)
// ===================================================================
#define LORA_FREQ         433.175 // Frekuensi tengah legal (Alokasi: 433,050 - 434,790 MHz)
#define LORA_BW           125.0   // Bandwidth kanal (kHz)
#define LORA_SF           9       // Spreading Factor (SF9: uji ketahanan hambatan bertahap)
#define LORA_CR           7       // Coding Rate 4/7 (CR = 3)
#define LORA_SYNC_WORD    0x12    // Sync Word Jaringan Privat eSOS (SX127X)

// Pengaturan Daya: 10 dBm untuk kepatuhan regulasi EIRP lapangan (Maks 12,15 dBm EIRP)
// Untuk uji laboratorium terisolasi dengan atenuator, dapat disetel hingga +17 dBm (PA_BOOST).
#define LORA_TX_POWER     10      // Conducted Power (dBm)

// ===================================================================
// 3. KONTRAK STRUKTUR PAYLOAD BINER MUTLAK
// ===================================================================
// Format Biner: Little-Endian (Arsitektur Xtensa 32-bit LX6 ESP32).
// Ukuran Total: Tepat 34 Bytes (Tanpa padding compiler).
struct __attribute__((packed)) TelemetryPayload {
    uint8_t schema_version; // Offset  0 | 1 byte  : Versi skema biner (Selalu 1)
    char node_code[8];      // Offset  1 | 8 bytes : Kode identitas node ("WC_01\0\0\0")
    uint32_t sequence_no;   // Offset  9 | 4 bytes : Nomor urut paket monotonik (Little-Endian)
    uint32_t uptime_seconds;// Offset 13 | 4 bytes : Waktu operasional sejak boot (Detik, bukan Unix epoch)
    float water_level_cm;   // Offset 17 | 4 bytes : Nilai simulasi sensor air tangki (IEEE-754 Little-Endian)
    float ammonia_ppm;      // Offset 21 | 4 bytes : Nilai simulasi gas amonia (IEEE-754 Little-Endian)
    float h2s_ppm;          // Offset 25 | 4 bytes : Nilai simulasi gas H2S (IEEE-754 Little-Endian)
    float battery_voltage;  // Offset 29 | 4 bytes : Nilai simulasi tegangan baterai 18650 (IEEE-754 Little-Endian)
    uint8_t sos_triggered;  // Offset 33 | 1 byte  : Flag status darurat (0 = Normal, 1 = Darurat)
};

// Verifikasi integritas ukuran payload saat kompilasi
static_assert(sizeof(TelemetryPayload) == 34, "FATAL: Ukuran TelemetryPayload harus tepat 34 bytes!");

// ===================================================================
// 4. INSTANSIASI OBJEK HARDWARE RADIOLIB
// ===================================================================
// Koreksi Argumen: Argumen ke-4 Module(cs, irq, rst, gpio) adalah GPIO tambahan (DIO1),
// BUKAN pin MISO! MISO dikonfigurasi melalui objek SPIClass. Gunakan RADIOLIB_NC dan oper objek SPI.
SX1278 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO0, PIN_LORA_RESET, RADIOLIB_NC, SPI);

// Handle Antrean FreeRTOS
QueueHandle_t xLoRaQueue = NULL;
uint32_t global_packet_counter = 0;

// ===================================================================
// TASK 1: PRODUCER SIMULASI TELEMETRI (Berjalan di Core 0)
// ===================================================================
void vTaskGenerateTelemetry(void *pvParameters) {
    for (;;) {
        TelemetryPayload packet;
        memset(&packet, 0, sizeof(TelemetryPayload));

        packet.schema_version = 1;
        strncpy(packet.node_code, "WC_01", sizeof(packet.node_code) - 1);
        packet.sequence_no = ++global_packet_counter;
        packet.uptime_seconds = (uint32_t)(millis() / 1000);

        // Nilai simulasi eksplisit (Bukan data sensor riil terkalibrasi)
        // Sumber daya BoM acuan: Baterai Li-ion 18650 1S4P (3.7V nominal, 4.2V max)
        packet.water_level_cm = 42.5f;   // Simulasi level tangki (Belum sensor fisik)
        packet.ammonia_ppm = 8.4f;       // Simulasi amonia (Belum sensor fisik)
        packet.h2s_ppm = 1.2f;           // Simulasi H2S (Belum sensor fisik)
        packet.battery_voltage = 3.82f;  // Simulasi Li-ion 18650 (Belum voltage divider riil)
        packet.sos_triggered = 0;

        UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[CORE 0 - PRODUCER] Paket #%u dibuat (Ukuran: %u bytes, Uptime: %u s, Stack Free: %u words)\n", 
                      packet.sequence_no, (unsigned int)sizeof(TelemetryPayload), packet.uptime_seconds, (unsigned int)stackRemaining);

        // Kirim ke antrean untuk dieksekusi oleh LoRa Task di Core 1
        if (xQueueSend(xLoRaQueue, &packet, pdMS_TO_TICKS(1000)) != pdTRUE) {
            Serial.printf("[CORE 0 - PERINGATAN] Antrean penuh! Paket #%u gagal dimasukkan.\n", packet.sequence_no);
        }

        // Interval pengujian: 5000 ms (Melepaskan CPU secara sukarela via FreeRTOS scheduler)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ===================================================================
// TASK 2: TRANSMISI RADIO LORA Ra-02 (Berjalan di Core 1)
// ===================================================================
void vTaskLoRaTransmitter(void *pvParameters) {
    TelemetryPayload txData;

    for (;;) {
        // Blokir (Sleep) tanpa membebani CPU sampai ada data masuk di antrean
        if (xQueueReceive(xLoRaQueue, &txData, portMAX_DELAY) == pdTRUE) {
            
            UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
            
            // Hitung estimasi teoritis Time-on-Air (ToA) berdasarkan register modulasi RadioLib
            float estimatedToA = radio.getTimeOnAir(sizeof(TelemetryPayload)) / 1000.0f; // Konversi us ke ms

            Serial.println("-------------------------------------------------------------------");
            Serial.printf("[CORE 1 - LORA TX] Mengambil Paket #%u dari Queue (Stack Free: %u words)\n", 
                          txData.sequence_no, (unsigned int)stackRemaining);
            Serial.printf("  Payload Konten: Node=%s | Uptime=%u s | Air=%.1f cm | NH3=%.1f | H2S=%.1f | Batt=%.2f V\n",
                          txData.node_code, txData.uptime_seconds, txData.water_level_cm, 
                          txData.ammonia_ppm, txData.h2s_ppm, txData.battery_voltage);
            Serial.printf("  RF Config     : Frek=%.3f MHz | SF=%d | BW=%.1f kHz | Daya=%d dBm\n",
                          LORA_FREQ, LORA_SF, LORA_BW, LORA_TX_POWER);
            Serial.printf("  Estimasi ToA  : %.2f ms (Dihitung dari formula Semtech SX1278)\n", estimatedToA);

            unsigned long startExecTime = millis();
            
            // Transmisi sinkronus biner via chip SX1278
            int txState = radio.transmit((uint8_t*)&txData, sizeof(TelemetryPayload));
            
            unsigned long measuredDuration = millis() - startExecTime;

            if (txState == RADIOLIB_ERR_NONE) {
                Serial.printf(">>> [TX SUKSES LOKAL] Sinyal TX_DONE diterima dari pin DIO0 (GPIO %d)!\n", PIN_LORA_DIO0);
                Serial.printf(">>> Durasi Pemanggilan Fungsi (SPI + ToA): %lu ms\n", measuredDuration);
                Serial.println(">>> CATATAN: Status ini HANYA menandakan transmisi lokal selesai dipancarkan.");
                Serial.println("    Keberhasilan penerimaan paket di sisi posko memerlukan verifikasi unit RX.");
            } else {
                Serial.printf(">>> [TX GAGAL] Kode Kesalahan RadioLib: %d\n", txState);
                Serial.println("    Periksa kestabilan catu daya 3.3V dan kontinuitas kabel pin DIO0.");
            }
            Serial.println("-------------------------------------------------------------------\n");
        }
    }
}

// ===================================================================
// SETUP (INISIALISASI SISTEM & DIAGNOSTIK)
// ===================================================================
void setup() {
    Serial.begin(115200);
    
    // Beri jeda 2 detik untuk stabilisasi Serial Monitor laptop
    delay(2000);

    Serial.println("\n\n===================================================================");
    Serial.println("   SMART-SANITATION eSOS - UJI LORA NODE TRANSMITTER (ESP32 DevKit V1) ");
    Serial.println("===================================================================");
    Serial.println("Parameter Teknis & Regulasi:");
    Serial.printf("  - Target Board        : DOIT ESP32 DevKit V1 (30-pin)\n");
    Serial.printf("  - Modul Radio         : Semtech SX1278 (Ai-Thinker Ra-02)\n");
    Serial.printf("  - Frekuensi Operasi   : %.3f MHz (Legal: 433,050 - 434,790 MHz)\n", LORA_FREQ);
    Serial.printf("  - Bandwidth Kanal     : %.1f kHz\n", LORA_BW);
    Serial.printf("  - Spreading Factor    : SF%d\n", LORA_SF);
    Serial.printf("  - Coding Rate         : 4/%d\n", LORA_CR);
    Serial.printf("  - Sync Word           : 0x%02X (Jaringan Privat)\n", LORA_SYNC_WORD);
    Serial.printf("  - Daya Pancar Chip    : +%d dBm (Target EIRP <= 12,15 dBm Kominfo)\n", LORA_TX_POWER);
    Serial.printf("  - Ukuran Struct Biner : %u Bytes (static_assert terverifikasi)\n", (unsigned int)sizeof(TelemetryPayload));
    Serial.println("===================================================================");

    // 1. Inisialisasi Bus SPI secara Eksplisit untuk DevKit V1
    Serial.print("1. Inisialisasi Hardware SPI Bus (SCK:18, MISO:19, MOSI:23, SS:5)... ");
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    Serial.println("[OK]");

    // Uji Diagnostik Langsung Jalur Fisik SPI Register 0x42 (RegVersion)
    pinMode(PIN_LORA_NSS, OUTPUT);
    digitalWrite(PIN_LORA_NSS, HIGH);
    delay(10);
    digitalWrite(PIN_LORA_NSS, LOW);
    SPI.transfer(0x42 & 0x7F); // Alamat 0x42, bit MSB 0 untuk Read
    uint8_t rawVersion = SPI.transfer(0x00);
    digitalWrite(PIN_LORA_NSS, HIGH);

    Serial.printf("   [DIAGNOSTIK FISIK SPI] Pembacaan Langsung RegVersion (0x42): 0x%02X\n", rawVersion);
    if (rawVersion == 0x12) {
        Serial.println("   -> [STATUS SPI] Chip SX1278 merespons normal (0x12)! Jalur SPI dan VCC berfungsi.");
    } else if (rawVersion == 0x00) {
        Serial.println("   -> [ANALISIS 0x00] Jalur MISO selalu LOW! Periksa:");
        Serial.println("      * Modul Ra-02 TIDAK mendapatkan tegangan 3.3V (Kabel VCC/GND putus atau rel breadboard terputus di tengah).");
        Serial.println("      * Pin RST (GPIO 14) terhubung ke GND atau tertahan LOW (Chip dalam kondisi Reset).");
    } else if (rawVersion == 0xFF) {
        Serial.println("   -> [ANALISIS 0xFF] Jalur MISO selalu HIGH / Mengambang! Periksa:");
        Serial.println("      * Pin NSS (GPIO 5) TIDAK terhubung ke pin NSS modul Ra-02 (Chip tidak ter-select).");
        Serial.println("      * Pin MOSI (GPIO 23) atau SCK (GPIO 18) tidak terhubung.");
        Serial.println("      * Pin MOSI dan MISO TERTUKAR (MOSI disambung ke MISO, dsb).");
    } else {
        Serial.printf("   -> [ANALISIS 0x%02X] Nilai acak! Sinyal SPI tidak stabil / jumper longgar.\n", rawVersion);
    }

    // 2. Inisialisasi Radio SX1278
    Serial.print("2. Menghubungi Register Chip Semtech SX1278... ");
    int initState = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);

    if (initState == RADIOLIB_ERR_NONE) {
        Serial.println("[OK]");
        Serial.println("   -> Chip SX1278 merespons normal pada bus SPI.");
        Serial.println("   -> PERINGATAN FISIK: Pastikan antena 433 MHz terpasang kencang!");
        Serial.println("      Respon SPI TIDAK mendeteksi ada/tidaknya antena fisik.");
        
        // Aktifkan hardware CRC secara eksplisit agar cocok dengan sisi penerima
        int crcState = radio.setCRC(true);
        if (crcState != RADIOLIB_ERR_NONE) {
            Serial.printf("   -> [PERINGATAN] Gagal mengaktifkan CRC hardware: %d\n", crcState);
        } else {
            Serial.println("   -> Hardware CRC: [AKTIF]");
        }
    } else {
        Serial.println("[GAGAL]");
        Serial.printf("   -> Kode Galat RadioLib: %d\n", initState);
        Serial.println("   -> Langkah Diagnostik:");
        Serial.println("      1. Periksa kabel SPI (MOSI:23, MISO:19, SCK:18, NSS:5, RST:14).");
        Serial.println("      2. Pastikan tegangan VCC adalah 3.3V stabil (Bukan 5V).");
        Serial.println("      3. Periksa pin DIO0 (GPIO 2) tidak tertahan tegangan tinggi saat boot.");
        Serial.println("Sistem dihentikan.");
        while (true) delay(1000);
    }

    // 3. Alokasi Antrean FreeRTOS
    Serial.print("3. Mengalokasikan FreeRTOS Queue (Kapasitas: 5 Paket)... ");
    xLoRaQueue = xQueueCreate(5, sizeof(TelemetryPayload));
    if (xLoRaQueue == NULL) {
        Serial.println("[GAGAL]");
        Serial.println("   -> Alokasi memori heap untuk Queue gagal! Sistem berhenti.");
        while (true) delay(1000);
    }
    Serial.println("[OK]");

    // 4. Pembagian Multitasking FreeRTOS ke Dual-Core
    Serial.println("4. Memasang Task FreeRTOS ke Dual Core ESP32:");
    
    BaseType_t taskSensorsStatus = xTaskCreatePinnedToCore(
        vTaskGenerateTelemetry, 
        "TaskSensors", 
        3072,   // Stack 3072 words
        NULL, 
        1,      // Prioritas 1
        NULL, 
        0       // Core 0 (PRO_CPU)
    );

    BaseType_t taskLoRaStatus = xTaskCreatePinnedToCore(
        vTaskLoRaTransmitter, 
        "TaskLoRaTx", 
        4096,   // Stack 4096 words
        NULL, 
        2,      // Prioritas 2
        NULL, 
        1       // Core 1 (APP_CPU)
    );

    if (taskSensorsStatus != pdPASS || taskLoRaStatus != pdPASS) {
        Serial.println("[GAGAL]");
        Serial.println("   -> Gagal membuat salah satu Task FreeRTOS! Periksa sisa heap.");
        while (true) delay(1000);
    }
    Serial.println("   -> TaskSensors (Core 0, Prio 1) : [BERHASIL DIBUAT]");
    Serial.println("   -> TaskLoRaTx  (Core 1, Prio 2) : [BERHASIL DIBUAT]");
    Serial.println("      (Pesan ini menandakan alokasi task berhasil, bukan kepastian transmisi berulang)");
    Serial.println("===================================================================\n");
}

// ===================================================================
// LOOP FUNCTION (Perilaku FreeRTOS pada Arduino-ESP32)
// ===================================================================
void loop() {
    // Pada Arduino-ESP32, loop() berjalan di dalam loopTask bawaan.
    // Menghapus task ini dengan vTaskDelete(NULL) akan menghentikan eksekusi loopTask
    // dan membiarkan Task Idle FreeRTOS mereklamasi memori stack-nya secara asinkron.
    vTaskDelete(NULL);
}
