#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <Adafruit_ADXL345_U.h>
#include <SPI.h>
#include <LoRa.h>

// DHT11 Sensor Setup
#define DHTPIN 7          // DHT11 data pin
#define DHTTYPE DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// BMP280 Sensor Setup
Adafruit_BMP280 bmp;       // I2C Interface

// ADXL345 Sensor Setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// LoRa RA-02 Setup
const long frequency = 915E6;  // LoRa Frequency
const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // LoRa radio interrupt pin

void setup() {
  Serial.begin(9600);

  // Initialize DHT11
  dht.begin();

  // Initialize BMP280
  if (!bmp.begin(0x76)) {   // Check BMP280 address, 0x76 or 0x77
    Serial.println("BMP280 not detected, check wiring!");
    while (1);
  }

  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("ADXL345 not detected, check wiring!");
    while (1);
  }

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (1);
  }
  Serial.println("LoRa init succeeded.");
}

void loop() {
  // Reading DHT11 Sensor
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Reading BMP280 Sensor
  float temperatureBMP = bmp.readTemperature();
  float pressure = bmp.readPressure();

  // Reading ADXL345 Sensor
  sensors_event_t event;
  accel.getEvent(&event);

  // Prepare data to send
  String data = String("DHT11 Temp: ") + temperatureDHT + " *C, Hum: " + humidity + " %\n";
  data += String("BMP280 Temp: ") + temperatureBMP + " *C, Pressure: " + (pressure / 100.0F) + " hPa\n";
  data += String("ADXL345 Accel -> X: ") + event.acceleration.x + " m/s^2, Y: " + event.acceleration.y + " m/s^2, Z: " + event.acceleration.z + " m/s^2\n";

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();

  // Print data to Serial
  Serial.println("Sent via LoRa:");
  Serial.println(data);

  delay(5000); // Delay between readings
}
