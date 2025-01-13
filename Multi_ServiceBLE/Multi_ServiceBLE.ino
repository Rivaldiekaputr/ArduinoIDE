#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// UUID untuk Service 1 dan karakteristiknya
#define SERVICE_UUID1        "4fafc201-1fb5-459e-8fcc-c5c9c331914a"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a3"

// UUID untuk Service 2 dan karakteristiknya
#define SERVICE_UUID2        "6d4f2b10-b5a3-4bc4-bc3d-9da45c2d5d78"
#define CHARACTERISTIC_UUID2 "1e63f8f3-78aa-48be-a091-1d1c554d06c9"

// Variabel untuk menyimpan pointer karakteristik (untuk notifikasi)
BLECharacteristic *pCharacteristic1;
BLECharacteristic *pCharacteristic2;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server with multiple services and notifications!");

  BLEDevice::init("Multi-Service BLE");

  // Membuat server BLE
  BLEServer *pServer = BLEDevice::createServer();

  // Membuat Service 1
  BLEService *pService1 = pServer->createService(SERVICE_UUID1);
  pCharacteristic1 = pService1->createCharacteristic(
    CHARACTERISTIC_UUID1,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic1->setValue("Hello from Service 1");
  pService1->start();

  // Membuat Service 2
  BLEService *pService2 = pServer->createService(SERVICE_UUID2);
  pCharacteristic2 = pService2->createCharacteristic(
    CHARACTERISTIC_UUID2,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic2->setValue("Hello from Service 2");
  pService2->start();

  // Memulai iklan BLE
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID1);
  pAdvertising->addServiceUUID(SERVICE_UUID2);
  pAdvertising->start();
  Serial.println("Services are now advertised!");
}

void loop() {
  // Kirim notifikasi pada Service 1
  String message1 = "Service 1 Time: " + String(millis() / 1000) + " seconds";
  pCharacteristic1->setValue(message1.c_str());
  pCharacteristic1->notify();
  Serial.println("Notifying Service 1: " + message1);

  // Kirim notifikasi pada Service 2
  String message2 = "Service 2 Time: " + String(millis() / 1000) + " seconds";
  pCharacteristic2->setValue(message2.c_str());
  pCharacteristic2->notify();
  Serial.println("Notifying Service 2: " + message2);

  delay(2000); // Delay untuk interval notifikasi
}
