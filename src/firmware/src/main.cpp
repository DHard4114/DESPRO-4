/**
 * Smart-Sanitation eSOS — ESP32 Sensor Node & LoRa Telemetry Transmitter
 * Author: Siti Amalia Nurfaidah (Software Developer)
 * Course: Desain Proyek 2 (DTE FTUI Gasal 2026/2027)
 */

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include "config.h"

// Object Instances
Servo valveServo;

// State Variables
unsigned long lastSampleTime = 0;
unsigned long lastTxTime = 0;
unsigned long packetSequence = 0;
bool sosActive = false;
bool servoState = false;

// Function Prototypes
float readUltrasonicDistance();
float readGasAnalogPpm(int pin, float r0, float loadResistor);
float readBatteryVoltage();
void transmitLoRaPacket(float distance, float nh3, float h2s, float volt, bool sos);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[INIT] Starting Smart-Sanitation eSOS Firmware v1.0...");

    // Pin Modes
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STATUS_LED_PIN, OUTPUT);

    // Attach Servo
    ESP32PWM::allocateTimer(0);
    valveServo.setPeriodHertz(50);
    valveServo.attach(SERVO_PWM_PIN, 500, 2400);
    valveServo.write(0); // Valve Closed initially

    // Initialize LoRa SX1278
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("[ERROR] LoRa init failed! Check connections.");
        while (1) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(200);
        }
    }

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setSyncWord(0x12);
    LoRa.enableCrc();
    LoRa.setTxPower(17);

    Serial.println("[SUCCESS] LoRa Initialized @ 433 MHz.");
    digitalWrite(STATUS_LED_PIN, HIGH);
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. Check SOS Button Trigger (Instant Interrupt Check)
    if (digitalRead(SOS_BUTTON_PIN) == LOW) {
        sosActive = true;
        digitalWrite(STATUS_LED_PIN, LOW); // Fast blink indication
    } else {
        sosActive = false;
        digitalWrite(STATUS_LED_PIN, HIGH);
    }

    // 2. Periodic Sampling and Transmission
    if (currentMillis - lastTxTime >= LORA_TX_INTERVAL_MS) {
        lastTxTime = currentMillis;

        float waterLevelCm = readUltrasonicDistance();
        float ammoniaPpm = readGasAnalogPpm(MQ137_ANALOG_PIN, 32.5, 47.0);
        float h2sPpm = readGasAnalogPpm(MQ136_ANALOG_PIN, 15.8, 20.0);
        float batteryVolt = readBatteryVoltage();

        // Transmit Packet
        transmitLoRaPacket(waterLevelCm, ammoniaPpm, h2sPpm, batteryVolt, sosActive);
    }
}

float readUltrasonicDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 100.0; // Default max distance on timeout

    float distanceCm = (duration * 0.0343) / 2.0;
    return distanceCm;
}

float readGasAnalogPpm(int pin, float r0, float loadResistor) {
    long rawSum = 0;
    for (int i = 0; i < NUM_ADC_SAMPLES; i++) {
        rawSum += analogRead(pin);
        delay(2);
    }
    float avgRaw = rawSum / (float)NUM_ADC_SAMPLES;
    float voltage = (avgRaw / 4095.0) * 3.3;

    if (voltage <= 0.1) return 0.0;
    float rs = ((3.3 - voltage) / voltage) * loadResistor;
    float ratio = rs / r0;

    // Approximated Power Law curve: ppm = a * (Rs/Ro)^b
    float ppm = 10.0 * pow(ratio, -1.45);
    return ppm;
}

float readBatteryVoltage() {
    long rawSum = 0;
    for (int i = 0; i < NUM_ADC_SAMPLES; i++) {
        rawSum += analogRead(BATTERY_VOLT_PIN);
        delay(2);
    }
    float avgRaw = rawSum / (float)NUM_ADC_SAMPLES;
    float pinVolt = (avgRaw / 4095.0) * 3.3;
    // Voltage divider scaling (100k + 20k) / 20k = 6.0
    float actualVolt = pinVolt * (120.0 / 20.0);
    return actualVolt;
}

void transmitLoRaPacket(float distance, float nh3, float h2s, float volt, bool sos) {
    packetSequence++;

    StaticJsonDocument<256> doc;
    doc["nodeId"] = NODE_ID;
    doc["seq"] = packetSequence;
    doc["waterCm"] = round(distance * 10) / 10.0;
    doc["nh3"] = round(nh3 * 10) / 10.0;
    doc["h2s"] = round(h2s * 10) / 10.0;
    doc["batVolt"] = round(volt * 100) / 100.0;
    doc["sos"] = sos ? 1 : 0;

    String jsonString;
    serializeJson(doc, jsonString);

    LoRa.beginPacket();
    LoRa.print(jsonString);
    LoRa.endPacket();

    Serial.print("[TX] Sequence #");
    Serial.print(packetSequence);
    Serial.print(" | Payload: ");
    Serial.println(jsonString);
}
