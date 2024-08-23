#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <Adafruit_ADXL345_U.h>

// DHT11 Sensor Setup
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// BMP280 Sensor Setup
Adafruit_BMP280 bmp;

// ADXL345 Sensor Setup
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Number of samples for averaging
const int numSamples = 10;

// Low-pass filter constants
const float alpha = 0.2;
float xFiltered = 0;
float yFiltered = 0;
float zFiltered = 0;

// Calibration offsets
const float xOffset = 0.0; // Adjust based on calibration
const float yOffset = 0.0; // Adjust based on calibration
const float zOffset = 9.81; // Gravity should be close to 9.81 m/s²

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not detected, check wiring!");
    while (1);
  }

  if (!accel.begin()) {
    Serial.println("ADXL345 not detected, check wiring!");
    while (1);
  }
}

void readAndPrintADXL345() {
  sensors_event_t event;
  accel.getEvent(&event);

  if (isnan(event.acceleration.x) || isnan(event.acceleration.y) || isnan(event.acceleration.z)) {
    Serial.println("Failed to read from ADXL345 sensor!");
    return;
  }

  // Averaging raw data
  float averageX = 0;
  float averageY = 0;
  float averageZ = 0;
  for (int i = 0; i < numSamples; i++) {
    averageX += event.acceleration.x;
    averageY += event.acceleration.y;
    averageZ += event.acceleration.z;
    delay(10);
  }
  averageX /= numSamples;
  averageY /= numSamples;
  averageZ /= numSamples;

  // Print raw accelerometer values
  Serial.print("Raw Accel X: ");
  Serial.print(averageX, 2);
  Serial.print(" m/s², Y: ");
  Serial.print(averageY, 2);
  Serial.print(" m/s², Z: ");
  Serial.print(averageZ, 2);
  Serial.println(" m/s²");

  // Apply low-pass filter
  xFiltered = alpha * (averageX - xOffset) + (1 - alpha) * xFiltered;
  yFiltered = alpha * (averageY - yOffset) + (1 - alpha) * yFiltered;
  zFiltered = alpha * (averageZ - zOffset) + (1 - alpha) * zFiltered;

  // Print filtered accelerometer values
  Serial.print("Filtered Accel X: ");
  Serial.print(xFiltered, 2);
  Serial.print(" m/s², Y: ");
  Serial.print(yFiltered, 2);
  Serial.print(" m/s², Z: ");
  Serial.print(zFiltered, 2);
  Serial.println(" m/s²");
}

void loop() {
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();
  float temperatureBMP = bmp.readTemperature();
  float pressure = bmp.readPressure();

  readAndPrintADXL345();

  String data = "DHT11 Temp: " + String(temperatureDHT, 1) + " C, Hum: " + String(humidity, 1) + " % | ";
  data += "BMP280 Temp: " + String(temperatureBMP, 1) + " C, Pressure: " + String(pressure / 100.0F, 1) + " hPa";

  Serial.println("Data:");
  Serial.println(data);

  delay(1000);
}
