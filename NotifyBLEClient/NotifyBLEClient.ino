#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914c" // UUID layanan yang sama dengan server
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7" // UUID karakteristik yang sama dengan server

bool connected = false; // Status apakah klien terhubung ke server
BLEAdvertisedDevice* myDevice = nullptr; // Menyimpan perangkat yang diiklankan

// Deklarasi prototipe fungsi scan
void scan();

// Callback untuk menerima notifikasi dari server
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
        Serial.println("Terhubung ke server BLE!");
        connected = true;
    }

    void onDisconnect(BLEClient* pClient) {
        Serial.println("Terputus dari server BLE.");
        connected = false;
        scan();
    }
};

// Callback untuk menangani perangkat yang diiklankan saat scanning
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.print("Ditemukan perangkat BLE: ");
        Serial.println(advertisedDevice.toString().c_str());

        // Jika perangkat mengiklankan layanan dengan UUID yang sesuai
        if (advertisedDevice.haveServiceUUID() && 
            advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
            Serial.println("Server BLE dengan layanan yang sesuai ditemukan!");
            myDevice = new BLEAdvertisedDevice(advertisedDevice); // Simpan perangkat
            BLEDevice::getScan()->stop(); // Hentikan scanning
        }
    }
};

bool connectToServer() {
    Serial.println("Menghubungkan ke server BLE...");

    // Membuat klien BLE
    BLEClient* pClient = BLEDevice::createClient();
    Serial.println("Klien BLE dibuat.");
    pClient->setClientCallbacks(new MyClientCallback()); // Mengatur callback klien

    // Menghubungkan ke perangkat server
    if (!pClient->connect(myDevice)) {
        Serial.println("Gagal terhubung ke server BLE.");
        return false;
    }
    Serial.println("Terhubung ke server BLE!");

    // Mendapatkan layanan dari server berdasarkan UUID
    BLERemoteService* pService = pClient->getService(BLEUUID(SERVICE_UUID));
    if (pService == nullptr) {
        Serial.println("Layanan tidak ditemukan di server BLE.");
        pClient->disconnect();
        return false;
    }
    Serial.println("Layanan ditemukan.");

    // Mendapatkan karakteristik dari layanan berdasarkan UUID
    BLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
    if (pCharacteristic == nullptr) {
        Serial.println("Karakteristik tidak ditemukan di layanan.");
        pClient->disconnect();
        return false;
    }
    Serial.println("Karakteristik ditemukan.");

    // Memulai notifikasi untuk karakteristik
    if (pCharacteristic->canNotify()) {
            for (int i = 0; i < 100; i++) {
                Serial.print("Data notifikasi diterima: ");
                Serial.println(i); // Menampilkan data dalam format integer
                delay(1000);
            }
            Serial.println();
    }
    return true;
}


void setup() {
    Serial.begin(115200); // Memulai komunikasi serial untuk debug
    Serial.println("Memulai klien BLE...");

    // Inisialisasi perangkat BLE
    BLEDevice::init("");

    // Memulai scanning untuk menemukan perangkat BLE
   scan();
}

void loop() {
    // Jika perangkat yang diiklankan ditemukan, mencoba terhubung ke server
    if (myDevice != nullptr) {
        if (connectToServer()) {
            Serial.println("Koneksi berhasil, menerima notifikasi...");
        } else {
            Serial.println("Koneksi gagal, mencoba lagi...");
        }
        myDevice = nullptr; // Reset perangkat setelah mencoba koneksi
    }

    delay(1000); // Menunggu untuk mencegah pengulangan terlalu cepat
}

void scan(){
  Serial.println("Scan again.......");
   BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); // Mengatur callback scanning
    pBLEScan->setInterval(100); // Interval scanning dalam ms
    pBLEScan->setWindow(99); // Window scanning dalam ms
    pBLEScan->setActiveScan(true); // Scan aktif (mendapatkan data lebih banyak)
    pBLEScan->start(30, false); // Memulai scanning selama 30 detik
}

