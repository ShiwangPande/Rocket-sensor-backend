#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"

// BMP280 setup
Adafruit_BMP280 bmp; // I2C interface

// DHT11 setup
#define DHTPIN 7 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  Serial.println(F("Sensor Test"));

  // Initialize BMP280
  if (!bmp.begin(0x76)) { // Default I2C address
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    while (1) delay(10);
  }

  // Initialize DHT11
  dht.begin();
}

void loop() {
  // Read and print BMP280 data
  Serial.println(F("BMP280 Sensor Data:"));
  
  Serial.print(F("Temperature = "));
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(1013.25)); // Adjust the baseline pressure as needed
  Serial.println(" m");

  // Read and print DHT11 data
  Serial.println(F("\nDHT11 Sensor Data:"));
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  Serial.println();
  delay(2000); // Delay between readings
}
