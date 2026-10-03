#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan *scanner;
BLEAddress hubAddress("00:00:00:00:00:00");
BLERemoteCharacteristic* lwp3 = nullptr;
std::map<int, String> deviceIDs = {
  {21, "Hub Battery Current"},
  {20, "Hub Battery Voltage"},
  {57, "Hub IMU Accelerometer"},
  {54, "Hub IMU Gesture"},
  {58, "Hub IMU Gyroscope"},
  {59, "Hub IMU Orientation"},
  {60, "Hub IMU Temperature"},
  {22, "Hub Piezoelectric Buzzer"},
  {56, "Hub Bluetooth RSSI"},
  {23, "PUP Hub Status Light"},
  {8, "PUP Light"},
  {1, "PUP Medium Motor"},
  {55, "PUP Remote"},
  {76, "Technic Large Servo"},
  {46, "Technic Large Motor"},
  {75, "Technic Medium Servo"},
  {47, "Technic XL Motor"}
};
int ports[4] = {};
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

void notify_call(BLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
    for (int i = 0; i < length; i++) {
      if (i == 4) {
        if (data[i] == 0) {
          if (data[3] < 4 && data[3] >= 0) {
            Serial.print(deviceIDs[ports[data[3]]]);
            Serial.print(" disconnected from port number ");
            Serial.print(data[3]);
            Serial.println(".");
          } else {
            Serial.print("Port ");
            Serial.print(data[3]);
            Serial.println(" disconnected.");
          }
          ports[data[3]] = 0;
        } else {
          ports[data[3]] = data[5];
          Serial.print(deviceIDs[data[5]]);
          Serial.print(" connected to port number ");
          Serial.print(data[3]);
          Serial.println(".");
        }
      }
    }
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
      BLERemoteService* service = client->getService(
        BLEUUID("00001623-1212-efde-1623-785feabcd123")
      );
      lwp3 = service->getCharacteristic(
        BLEUUID("00001624-1212-efde-1623-785feabcd123")
      );
      lwp3->registerForNotify(notify_call);
    }
    scanner->clearResults();
    delay(1000);
}