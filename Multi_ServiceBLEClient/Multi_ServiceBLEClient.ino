#include <BLEDevice.h>

// UUID untuk Service 1 dan karakteristiknya
#define SERVICE_UUID1        "4fafc201-1fb5-459e-8fcc-c5c9c331914a"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a3"

// UUID baru untuk digunakan oleh client
static BLEUUID serviceUUID1(SERVICE_UUID1);
static BLEUUID charUUID1(CHARACTERISTIC_UUID1);

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic1;
static BLEAdvertisedDevice* myDevice;

// Callback untuk notifikasi karakteristik
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notification received from ");
    Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print("Data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    Serial.println("Connected to the BLE server!");
  }

  void onDisconnect(BLEClient* pClient) {
    connected = false;
    Serial.println("Disconnected from the BLE server!");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  // Menghubungkan ke Service 1
  BLERemoteService* pRemoteService1 = pClient->getService(serviceUUID1);
  if (pRemoteService1 == nullptr) {
    Serial.println("Failed to find Service 1.");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found Service 1");
  pRemoteCharacteristic1 = pRemoteService1->getCharacteristic(charUUID1);
  if (pRemoteCharacteristic1 == nullptr) {
    Serial.println("Failed to find Characteristic 1.");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found Characteristic 1");
  if (pRemoteCharacteristic1->canNotify())
    pRemoteCharacteristic1->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Mencari perangkat dengan Service 1
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID1)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client application...");
  BLEDevice::init("");

  // Memulai scan untuk mencari perangkat
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1000);
  pBLEScan->setWindow(999);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  // Jika flag doConnect aktif, hubungkan ke server BLE
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE server.");
    } else {
      Serial.println("Failed to connect to the BLE server. Reconnect.....");
      
    }
    doConnect = false;
  }

  // Loop utama menunggu notifikasi
  delay(1000);
}
