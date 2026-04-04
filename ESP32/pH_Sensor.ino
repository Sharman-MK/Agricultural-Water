#include <Wire.h>

// Calibration value
float calibration_value = 21.34 + 1.5;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

// Tentukan pin analog untuk sensor pH
#define PH_SENSOR_PIN 34   // GPIO34 (ADC1)

void setup() {
  Wire.begin();
  Serial.begin(115200);   // Baudrate ESP32
  
  Serial.println("System Initializing...");
  Serial.println("pH Sensor Ready");
  delay(2000);
}

void loop() {
  // 1. Baca data analog
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(30);
  }

  // 2. Urutkan data (bubble sort) untuk menghilangkan noise
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // 3. Ambil rata-rata dari data tengah (index 2 sampai 7)
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];

  // 4. Konversi ke voltase ESP32 (3.3V / 4095 resolution)
  float volt = (float)avgval * 3.3 / 4095.0 / 6;
  ph_act = -5.70 * volt + calibration_value;

  // 5. Tampilkan di Serial Monitor
  Serial.print("--- Reading ---");
  Serial.print(" | Voltage: ");
  Serial.print(volt, 3);
  Serial.print("V | pH Value: ");
  Serial.println(ph_act, 2);

  delay(1000); // Delay 1 detik untuk kestabilan
}
