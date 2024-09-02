#include <SPI.h>
#include <LoRa.h>

// Pin Definitions
#define SS 10
#define RST 9
#define DI0 2
#define BAND 433E6

void setup() {
  // Start Serial communication
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  // Initialize LoRa module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  // Try to parse the packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      char c = LoRa.read();
      receivedData += c;
    }
    Serial.println(receivedData);
  }

  // Add a small delay to prevent excessive CPU usage
  delay(10);
}