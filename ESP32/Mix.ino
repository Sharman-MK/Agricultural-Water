#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>

// --- Pin Definitions ---
#define ONE_WIRE_BUS 2     // DS18B20 Data Pin
#define PH_SENSOR_PIN 34   // pH Sensor Analog Pin (ESP32 ADC1)

// --- Sensor Objects ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
Adafruit_ADS1115 ads;

// --- Global Variables & Calibration ---
float calibration_value = 21.34 + 1.5; // pH Calibration
float currentTempC = 25.0;            // Default temp for compensation

unsigned long lastReadTime = 0;
const int interval = 1500;            // Read every 1.5 seconds

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize DS18B20
  tempSensors.begin();
  
  // 2. Initialize ADS1115 (TDS)
  ads.setGain(GAIN_ONE); // +/- 4.096V
  if (!ads.begin()) {
    Serial.println("Warning: ADS1115 (TDS) not found!");
  }

  Serial.println("Multi-Sensor System Initialized");
  Serial.println("Temp(C) | pH | TDS(ppm)");
  Serial.println("---------------------------------");
}

void loop() {
  if (millis() - lastReadTime > interval) {
    lastReadTime = millis();

    // --- 1. DS18B20 TEMPERATURE READING ---
    tempSensors.requestTemperatures();
    float tempReading = tempSensors.getTempCByIndex(0);
    if (tempReading != DEVICE_DISCONNECTED_C) {
      currentTempC = tempReading;
    }

    // --- 2. pH SENSOR READING (With Noise Filtering) ---
    int buffer_arr[10], tempSwap;
    for (int i = 0; i < 10; i++) {
      buffer_arr[i] = analogRead(PH_SENSOR_PIN);
      delay(10); 
    }
    // Bubble Sort
    for (int i = 0; i < 9; i++) {
      for (int j = i + 1; j < 10; j++) {
        if (buffer_arr[i] > buffer_arr[j]) {
          tempSwap = buffer_arr[i];
          buffer_arr[i] = buffer_arr[j];
          buffer_arr[j] = tempSwap;
        }
      }
    }
    // Average middle 6 samples
    long avgval = 0;
    for (int i = 2; i < 8; i++) avgval += buffer_arr[i];
    float phVolt = (float)avgval * 3.3 / 4095.0 / 6.0;
    float phValue = -5.70 * phVolt + calibration_value;

    // --- 3. TDS SENSOR READING (Via ADS1115) ---
    int16_t adc0 = ads.readADC_SingleEnded(0);
    float tdsVolt = adc0 * 0.125 / 1000.0;
    
    // Automatic Temperature Compensation using DS18B20 data
    float compensationCoefficient = 1.0 + 0.02 * (currentTempC - 25.0);
    float compensationVoltage = tdsVolt / compensationCoefficient;
    
    // TDS Formula
    float tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;

    // --- 4. OUTPUT RESULTS ---
    Serial.print("Temp: ");
    Serial.print(currentTempC, 1);
    Serial.print("°C | pH: ");
    Serial.print(phValue, 2);
    Serial.print(" | TDS: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");
  }
}
