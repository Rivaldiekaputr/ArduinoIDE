#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

// Definisi pin dan tipe sensor DHT
#define DHTPIN 32  // Pin untuk sensor DHT11
#define DHTTYPE DHT11

void advertising();

// Inisialisasi objek DHT
DHT dht(DHTPIN, DHTTYPE);

// UUID untuk Layanan dan Karakteristik BLE
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define TEMP_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"   // UUID untuk suhu
#define HUMID_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // UUID untuk kelembaban

// Variabel untuk menyimpan suhu dan kelembaban
float temperature = 0.0;
float humidity = 0.0;

BLEServer* pServer = NULL;
BLECharacteristic* pTempCharacteristic = NULL;
BLECharacteristic* pHumidCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Kelas untuk menangani koneksi dan pemutusan koneksi BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    advertising();
  }
};

void advertising() {
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->start();

  Serial.println("Menunggu koneksi dari klien...");
}
void setup() {
  Serial.begin(115200);
  dht.begin();  // Memulai sensor DHT11

  // Inisialisasi perangkat BLE
  BLEDevice::init("ESP32_DHT11_Server");

  // Membuat server BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Membuat layanan BLE
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Membuat karakteristik untuk suhu
  pTempCharacteristic = pService->createCharacteristic(
    TEMP_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTempCharacteristic->addDescriptor(new BLE2902());

  // Membuat karakteristik untuk kelembaban
  pHumidCharacteristic = pService->createCharacteristic(
    HUMID_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pHumidCharacteristic->addDescriptor(new BLE2902());

  // Memulai layanan BLE
  pService->start();

  // Memulai advertising
  advertising();
}

void loop() {
  // Membaca data dari sensor DHT11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Mengecek apakah pembacaan sensor berhasil
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca sensor DHT11!");
    delay(1000);
    return;
  }

  // Mengirimkan data suhu dan kelembaban jika perangkat terhubung
  if (deviceConnected) {
    Serial.println("Mengirim ke Client....");
    pTempCharacteristic->setValue(temperature);
    pTempCharacteristic->notify();  // Notifikasi suhu

    pHumidCharacteristic->setValue(humidity);
    pHumidCharacteristic->notify();  // Notifikasi kelembaban
    Serial.println("==================================");
  }

  delay(2000);  // Delay antara pembacaan sensor
}
