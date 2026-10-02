#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan *scanner;
BLEAddress hubAddress("00:00:00:00:00:00");
bool connected = false;
bool hubFound = false;

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice device) {
        if (device.haveName() && device.getName() == "Technic Hub") {
          Serial.print("Hub found! Address: ");
          Serial.print(device.getAddress().toString().c_str());
          Serial.print(" RSSI: ");
          Serial.println(device.getRSSI());
          hubFound = true;
          hubAddress = device.getAddress();
        }
    }
};

void get_characteristics(BLEClient* client) {
  std::map<std::string, BLERemoteService*>* services = client->getServices();
  for (auto &const entry : *services) {
    BLERemoteService* service = entry.second;
    Serial.print("Service: ");
    Serial.println(service->getUUID().toString().c_str());
  }
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting BLE scan...");

    BLEDevice::init("");

    scanner = BLEDevice::getScan();
    scanner->setAdvertisedDeviceCallbacks(
        new AdvertisedDeviceCallbacks()
    );

    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(99);

    Serial.println("Scanning...");
}

void loop() {
    BLEScanResults results = scanner->start(5, false);
    if (hubFound and !connected) {
      BLEClient* client = BLEDevice::createClient();
      if (client->connect(hubAddress)) {
        Serial.println("Connected successfully!");
        connected = true;
      } else {
        Serial.println("Failed to connect.");
      }
    }
    scanner->clearResults();
    delay(1000);
}