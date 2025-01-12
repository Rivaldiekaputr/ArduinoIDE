#include "BLEDevice.h"

// UUID untuk service dan karakteristik yang akan dihubungkan
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c3319149");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a7");

// Variabel status koneksi
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Fungsi callback saat menerima notifikasi dari server BLE
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Callback notifikasi untuk karakteristik ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" dengan panjang data ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

// Kelas callback untuk menangani koneksi dan pemutusan koneksi
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    // Fungsi ini akan dipanggil saat terhubung ke server BLE
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Terputus dari server BLE");
  }
};

// Fungsi untuk menghubungkan ke server BLE
bool connectToServer() {
    if (myDevice == nullptr) {
      Serial.println("Tidak ada perangkat yang ditemukan untuk dihubungkan.");
      return false;
    }

    Serial.print("Membentuk koneksi ke ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient* pClient = BLEDevice::createClient();
    Serial.println(" - Klien BLE dibuat");

    pClient->setClientCallbacks(new MyClientCallback());

    // Menghubungkan ke server BLE
    pClient->connect(myDevice);
    Serial.println(" - Terhubung ke server BLE");

    // Mendapatkan service dari server BLE
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Gagal menemukan service dengan UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Service ditemukan");

    // Mendapatkan karakteristik dari service
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Gagal menemukan karakteristik dengan UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Karakteristik ditemukan");

    // Membaca nilai karakteristik jika bisa dibaca
    if(pRemoteCharacteristic->canRead()) {
      String value = pRemoteCharacteristic->readValue();
      Serial.print("Nilai karakteristik: ");
      Serial.println(value.c_str());
    }

    // Mendaftar untuk notifikasi jika karakteristik mendukung notifikasi
    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

// Kelas callback untuk menangani perangkat BLE yang terdeteksi saat scanning
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Perangkat BLE ditemukan: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Mengecek apakah perangkat yang terdeteksi memiliki service yang dicari
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai aplikasi klien BLE di Arduino...");
  BLEDevice::init("");

  // Mengatur pemindai BLE dan callback saat perangkat terdeteksi
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  // Mengecek apakah ada permintaan koneksi ke server BLE
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Sekarang terhubung ke server BLE.");
    } else {
      Serial.println("Gagal terhubung ke server; tidak ada tindakan lebih lanjut.");
    }
    doConnect = false;
  }

  // Jika terhubung, kirim data ke karakteristik setiap detik
  if (connected) {
    String newValue = "Waktu sejak mulai: " + String(millis() / 1000);
    Serial.println("Mengatur nilai karakteristik baru: \"" + newValue + "\"");
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  } else if (doScan) {
    // Memulai kembali scanning jika terputus
    BLEDevice::getScan()->start(0);
    doScan = false;
  }
  
  delay(1000); // Menunggu 1 detik sebelum loop berikutnya
}
