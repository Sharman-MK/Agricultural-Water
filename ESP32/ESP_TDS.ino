#include <Wire.h>
#include <Adafruit_ADS1X15.h> // library from Adafruit ADS1X15

Adafruit_ADS1115 ads;

// --- Constants and Calibration ---
const float VREF = 3.3;      // Voltage reference of ADS1115 (set to match your VDD)
const float ADC_RANGE = 32767.0; // ADS1115 is 16-bit, but 15-bit for positive values
float temperature = 25.0;    // Default temperature for compensation

void setup() {
  Serial.begin(115200);
  
  // Initialize ADS1115
  // Gain can be adjusted: 
  // GAIN_ONE: +/- 4.096V (use this if powering ADS with 5V but sensor outputs <4V)
  // GAIN_TWO: +/- 2.048V (higher precision if sensor output is always low)
  ads.setGain(GAIN_ONE); 
  
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115!");
    while (1);
  }
  
  Serial.println("TDS Sensor with ADS1115 Initialized.");
}

void loop() {
  int16_t adc0;
  float voltage, compensationCoefficient, compensationVoltage, tdsValue;

  // 1. Read Raw Value from Channel 0
  adc0 = ads.readADC_SingleEnded(0);

  // 2. Convert raw ADC to Voltage
  // GAIN_ONE means 1 bit = 0.125mV
  voltage = adc0 * 0.125 / 1000.0;

  // 3. Temperature Compensation
  // TDS is highly sensitive to temperature (~2% per °C)
  compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  compensationVoltage = voltage / compensationCoefficient;

  // 4. Convert Voltage to TDS (Standard Formula)
  // TDS = (133.42 * voltage^3 - 255.86 * voltage^2 + 857.39 * voltage) * 0.5
  tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;

  // 5. Output to Serial
  Serial.print("ADC Raw: ");
  Serial.print(adc0);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V | TDS: ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");

  delay(1000);
}
