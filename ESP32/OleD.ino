#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <U8g2lib.h>

// ============================================================
// OLED DISPLAY
// ============================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ============================================================
// ADS1115
// ============================================================
Adafruit_ADS1115 ads;

// ============================================================
// FLOW SENSOR
// ============================================================
#define FLOW_PIN 27
volatile uint32_t flowPulses = 0;

// ============================================================
// ADS CHANNELS
// ============================================================
#define TDS_CH   0
#define PH_CH    1
#define PRESS_CH 2

// ============================================================
// VARIABLES
// ============================================================
float phValue = 0;
float tdsValue = 0;
float flowRate = 0;
float pressurePSI = 0;

// ============================================================
// CALIBRATION
// ============================================================
const float PH_SLOPE  = -5.70;
const float PH_OFFSET = 21.34;

const float FLOW_HZ_PER_LMIN = 7.5;

const float PRESS_MIN_V = 0.5;
const float PRESS_MAX_V = 4.5;
const float PRESS_MAX_PSI = 17.4;

// ============================================================
// ICONS (YOUR UI)
// ============================================================
const unsigned char icon_graph_el_am08fbebb_xbm[32] PROGMEM = {
  0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0xFB, 0xC0, 0xFB, 0xC0, 0xFB, 0xC0, 0xFB,
  0xDF, 0xFB, 0xDF, 0xFB, 0xDF, 0xFB, 0xDF, 0xFB, 0xDF, 0xFB, 0xDF, 0xFB, 0xDF, 0xFB, 0xDE, 0x7B
};

const unsigned char icon_gear_el_52zwlgqbo_xbm[32] PROGMEM = {
  0xC0, 0x03, 0xC8, 0x13, 0xDC, 0x3B, 0xFE, 0x7F, 0xFC, 0x3F, 0x38, 0x1C, 0x1F, 0xF8, 0x1F, 0xF8,
  0x1F, 0xF8, 0x1F, 0xF8, 0x38, 0x1C, 0xFC, 0x3F, 0xFE, 0x7F, 0xDC, 0x3B, 0xC8, 0x13, 0xC0, 0x03
};

const unsigned char icon_stat_el_xqnon1sw4_xbm[32] PROGMEM = {
  0xFE, 0x7F, 0xFF, 0xFF, 0x03, 0xC0, 0x03, 0xC1, 0x03, 0xC1, 0x83, 0xE2, 0x83, 0xE2, 0x8B, 0xD4,
  0x4B, 0xD4, 0x57, 0xC8, 0x57, 0xC8, 0x23, 0xC0, 0x23, 0xC0, 0x03, 0xC0, 0xFF, 0xFF, 0xFE, 0x7F
};

const unsigned char icon_nuke_el_zb3jdis5a_xbm[32] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x3C, 0x3C, 0x7E, 0x7E, 0xBE, 0x7D, 0xFE, 0x7F,
  0xC0, 0x03, 0x80, 0x01, 0xC0, 0x03, 0xC0, 0x03, 0xE0, 0x07, 0xE0, 0x07, 0xE0, 0x07, 0x00, 0x00
};

// ============================================================
// FLOW ISR
// ============================================================
void IRAM_ATTR flowISR() {
  flowPulses++;
}

// ============================================================
// READ VOLTAGE
// ============================================================
float readVoltage(uint8_t ch) {
  int16_t adc = ads.readADC_SingleEnded(ch);
  return ads.computeVolts(adc);
}

// ============================================================
// SENSOR UPDATE
// ============================================================
void readSensors() {

  // ---- pH ----
  float phVolt = readVoltage(PH_CH);
  phValue = constrain(PH_SLOPE * phVolt + PH_OFFSET, 0, 14);

  // ---- Pressure (F) ----
  float pVolt = readVoltage(PRESS_CH);
  float ratio = (pVolt - PRESS_MIN_V) / (PRESS_MAX_V - PRESS_MIN_V);
  pressurePSI = constrain(ratio * PRESS_MAX_PSI, 0, PRESS_MAX_PSI);

  // ---- Flow ----
  static uint32_t lastTime = millis();
  uint32_t now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  noInterrupts();
  uint32_t pulses = flowPulses;
  flowPulses = 0;
  interrupts();

  float freq = (dt > 0) ? (pulses / dt) : 0;
  flowRate = freq / FLOW_HZ_PER_LMIN;

  // ---- TDS ----
  float tdsVolt = readVoltage(TDS_CH);
  tdsValue = (133.42 * pow(tdsVolt, 3)
            - 255.86 * pow(tdsVolt, 2)
            + 857.39 * tdsVolt) * 0.5;
}

// ============================================================
// OLED UI (YOUR EXACT DESIGN)
// ============================================================
void drawHomeScreen() {

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawUTF8(30,8, "Water Meter");

  u8g2.drawLine(1, 9, 128, 9);

  // ICONS
  u8g2.drawXBMP(1, 11, 16, 16, icon_graph_el_am08fbebb_xbm);
  u8g2.drawXBMP(1, 29, 16, 16, icon_gear_el_52zwlgqbo_xbm);
  u8g2.drawXBMP(2, 47, 16, 16, icon_stat_el_xqnon1sw4_xbm);
  u8g2.drawXBMP(66, 28, 16, 16, icon_nuke_el_zb3jdis5a_xbm);

  // LABELS
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawUTF8(19, 25, "pH:");
  u8g2.drawUTF8(19, 43, "F:");
  u8g2.drawUTF8(20, 61, "Flow:");
  u8g2.drawUTF8(83, 43, "TDS:");

  // VALUES (aligned close to ":")
  u8g2.setCursor(38, 25);
  u8g2.print(phValue, 2);

  u8g2.setCursor(31, 43);
  u8g2.print(pressurePSI, 2);

  u8g2.setCursor(50, 61);
  u8g2.print(flowRate, 2);

  // TDS reverted (NOT too close)
  u8g2.setCursor(107, 43);
  u8g2.print(tdsValue, 0);

  u8g2.sendBuffer();
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFontMode(0);

  Wire.begin();

  if (!ads.begin()) {
    Serial.println("ADS1115 not found!");
    while (1);
  }

  ads.setGain(GAIN_TWOTHIRDS);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowISR, RISING);

  Serial.println("System Ready");
}

// ============================================================
// LOOP
// ============================================================
void loop() {

  static uint32_t lastUpdate = 0;

  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();

    readSensors();
    drawHomeScreen();

    Serial.println("------ DATA ------");
    Serial.print("pH: "); Serial.println(phValue);
    Serial.print("Pressure: "); Serial.println(pressurePSI);
    Serial.print("Flow: "); Serial.println(flowRate);
    Serial.print("TDS: "); Serial.println(tdsValue);
  }
}
