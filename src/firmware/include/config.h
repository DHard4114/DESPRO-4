/**
 * Smart-Sanitation eSOS Firmware Configuration Header
 * Hardware Pinout Mapping for ESP32 DevKitC 38-Pin
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Node Identification
#define NODE_ID "NODE_SANITATION_01"

// 1. LoRa SX1278 SPI Pins
#define LORA_SS_PIN     5
#define LORA_RST_PIN    14
#define LORA_DIO0_PIN   2
#define LORA_SCK_PIN    18
#define LORA_MISO_PIN   19
#define LORA_MOSI_PIN   23
#define LORA_FREQUENCY  433E6

// 2. Ultrasonic Sensor JSN-SR04T Pins
#define TRIG_PIN        12
#define ECHO_PIN        13

// 3. Analog Gas Sensors Pins (ADC1)
#define MQ137_ANALOG_PIN 34   // Ammonia (NH3)
#define MQ136_ANALOG_PIN 35   // Hydrogen Sulfide (H2S)
#define BATTERY_VOLT_PIN 36   // Voltage Divider (100k / 20k)

// 4. Actuator and Indicator Pins
#define SERVO_PWM_PIN    25   // MG996R Control Pin
#define SOS_BUTTON_PIN   27   // Emergency SOS Push Button (Active LOW)
#define STATUS_LED_PIN   26   // System Status Indicator LED

// Operational Constants
#define SAMPLE_INTERVAL_MS 5000   // Sensor Sampling Every 5 Seconds
#define LORA_TX_INTERVAL_MS 10000 // LoRa Transmit Every 10 Seconds
#define NUM_ADC_SAMPLES    16     // Moving Average Samples

#endif // CONFIG_H
