#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Initialize OLED (SSD1306 128x64 using I2C)
// U8G2_R0 = No rotation, [F]ull buffer mode, [HW] Hardware I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Initialize ADS1115
Adafruit_ADS1115 ads;

// Constants
float temperature = 25.0; 

void setup() {
  Serial.begin(115200);

  // Start I2C and OLED
  u8g2.begin();

  // Start ADS1115
  ads.setGain(GAIN_ONE); 
  if (!ads.begin()) {
    Serial.println("ADS1115 not found!");
    while (1);
  }
}

void loop() {
  // 1. Read from ADS1115 (Channel 0)
  int16_t adc0 = ads.readADC_SingleEnded(0);
  float voltage = adc0 * 0.125 / 1000.0;

  // 2. TDS Calculation Logic
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float compensationVoltage = voltage / compensationCoefficient;
  float tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;

  // 3. Draw to OLED
  u8g2.clearBuffer();					// Clear the internal memory

  // Draw the Main Outer Border
  u8g2.drawFrame(0, 0, 128, 64);        // Outer frame
  u8g2.drawFrame(2, 2, 124, 60);        // Inner frame for a "thick" border look

  // Draw Header
  u8g2.setFont(u8g2_font_6x10_tf);      // Small clean font
  u8g2.drawStr(35, 15, "WATER TDS");
  u8g2.drawLine(10, 18, 118, 18);       // Decorative line under title

  // Draw TDS Value
  u8g2.setFont(u8g2_font_helvB14_tr);   // Bold 14pt font
  u8g2.setCursor(15, 45);
  u8g2.print("TDS: ");
  u8g2.print((int)tdsValue);
  
  u8g2.setFont(u8g2_font_6x10_tf);      // Switch back to small font for units
  u8g2.print(" ppm");

  u8g2.sendBuffer();					// Transfer internal memory to the display

  delay(1000);
}
