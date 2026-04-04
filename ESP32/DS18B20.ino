#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 Sensor Configuration
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize DS18B20 Sensor
  sensors.begin();
  Serial.println("DS18B20 Temperature Sensor Initialized");
  Serial.println("Temperature (°C)");
  Serial.println("---------------------------");
}

void loop() {
  static unsigned long lastReadTime = 0;

  // Read temperature every second
  if (millis() - lastReadTime > 1000) {
    lastReadTime = millis();

    // Request temperature readings from the sensor
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    // Check if the sensor is actually connected
    if (temperatureC != DEVICE_DISCONNECTED_C) {
      // Display data on Serial Monitor
      Serial.print("Current Temp: ");
      Serial.print(temperatureC);
      Serial.println(" °C");
    } else {
      Serial.println("Error: Temperature Sensor Not Found!");
    }
  }
}
