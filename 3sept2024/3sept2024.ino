#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN 7
#define DHTTYPE DHT11
#define SS 10
#define RST 9
#define DI0 2
#define BAND 433E6

// Create sensor objects
Adafruit_BMP280 bmp;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
DHT dht(DHTPIN, DHTTYPE);

float startAltitude = 0;
float accelXOffset = 0;
float accelYOffset = 0;
float accelZOffset = 0;

// Define transmission interval
#define TRANSMISSION_INTERVAL 500  // Interval between transmissions (milliseconds)

// Function to prepare and send data
void sendDataOverLoRa(const String &data) {
  bool sendSuccess = false;
  LoRa.beginPacket();
  LoRa.print(data);
  for (int i = 0; i < 3; i++) {
    if (LoRa.endPacket(true)) {
      Serial.println("Packet sent successfully.");
      sendSuccess = true;
      break;
    } else {
      Serial.print("Failed to send packet, retrying... (Attempt ");
      Serial.print(i + 1);
      Serial.println(")");
      delay(200);  // Delay between retries
    }
  }
  if (!sendSuccess) {
    Serial.println("Error: Unable to send packet after multiple attempts.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize BMP280 sensor
  if (!bmp.begin(0x76)) {
    Serial.println("Error: BMP280 sensor not detected, check wiring!");
    while (1);  // Halt execution
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  startAltitude = bmp.readAltitude(1013.25);  // Set initial altitude

  // Initialize ADXL345 sensor
  if (!accel.begin()) {
    Serial.println("Error: ADXL345 sensor not detected, check wiring!");
    while (1);  // Halt execution
  }
  
  // Calibrate accelerometer
  Serial.println("Calibrating accelerometer...");
  float xSum = 0, ySum = 0, zSum = 0;
  const int calibrationSamples = 100;
  for (int i = 0; i < calibrationSamples; i++) {
    sensors_event_t event;
    accel.getEvent(&event);
    xSum += event.acceleration.x;
    ySum += event.acceleration.y;
    zSum += event.acceleration.z;
    delay(10);
  }
  accelXOffset = xSum / calibrationSamples;
  accelYOffset = ySum / calibrationSamples;
  accelZOffset = zSum / calibrationSamples;
  Serial.println("Calibration complete.");

  // Initialize LoRa module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Error: Starting LoRa failed!");
    while (1);  // Halt execution
  }

  Serial.println("Initialization complete.");
}

void loop() {
  // Read DHT11 sensor data
  float dhtTemperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check for valid DHT11 readings
  if (isnan(dhtTemperature) || isnan(humidity)) {
    Serial.println("Error: Failed to read from DHT sensor!");
    return;
  }

  // Read BMP280 sensor data
  float bmpTemperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1013.25) - startAltitude;

  // Check for BMP280 readings
  if (isnan(bmpTemperature) || isnan(pressure)) {
    Serial.println("Error: Failed to read from BMP280 sensor!");
    return;
  }

  // Read ADXL345 sensor data
  sensors_event_t event;
  accel.getEvent(&event);
  float accelX = event.acceleration.x - accelXOffset;
  float accelY = event.acceleration.y - accelYOffset;
  float accelZ = event.acceleration.z - accelZOffset;

  // Check for ADXL345 readings
  if (isnan(accelX) || isnan(accelY) || isnan(accelZ)) {
    Serial.println("Error: Failed to read from ADXL345 sensor!");
    return;
  }

  // Prepare the data string in JSON-like format
  String data = "{";
  data += "\"DHT_Temperature\":" + String(dhtTemperature, 2) + ",";
  data += "\"DHT_Humidity\":" + String(humidity, 2) + ",";
  data += "\"BMP_Temperature\":" + String(bmpTemperature, 2) + ",";
  data += "\"BMP_Pressure\":" + String(pressure / 100.0F, 2) + ",";
  data += "\"BMP_Altitude\":" + String(altitude, 2) + ",";
  data += "\"Accel_X\":" + String(accelX, 2) + ",";
  data += "\"Accel_Y\":" + String(accelY, 2) + ",";
  data += "\"Accel_Z\":" + String(accelZ, 2);
  data += "}";

  // Print data to Serial Monitor
  Serial.println("Data to send: " + data);

  // Send data over LoRa
  sendDataOverLoRa(data);

  delay(TRANSMISSION_INTERVAL);  // Transmit data at a specified interval
}
