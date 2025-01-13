#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Deklarasi variabel global
BLEServer* pServer = NULL;            // Objek server BLE
BLECharacteristic* pCharacteristic = NULL; // Objek karakteristik BLE
bool deviceConnected = false;         // Status apakah ada perangkat klien yang terhubung
bool oldDeviceConnected = false;      // Status koneksi sebelumnya
uint32_t value = 0;                   // Nilai yang akan dikirim melalui notifikasi

// UUID untuk layanan dan karakteristik BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"

// Deklarasi prototipe fungsi scan
void advertising();

// Kelas callback untuk menangani koneksi dan pemutusan koneksi
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true; // Set status terkoneksi ke true
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false; // Set status terkoneksi ke false
        advertising();
    }
};

void setup() {
    Serial.begin(115200); // Memulai komunikasi serial untuk debug

    // Inisialisasi perangkat BLE dengan nama "ESP32"
    BLEDevice::init("ESP32");

    // Membuat server BLE
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); // Mengatur callback server

    // Membuat layanan BLE dengan UUID tertentu
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Membuat karakteristik BLE
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID, // UUID karakteristik
        BLECharacteristic::PROPERTY_READ   | // Properti membaca data
        BLECharacteristic::PROPERTY_WRITE  | // Properti menulis data
        BLECharacteristic::PROPERTY_NOTIFY | // Properti notifikasi
        BLECharacteristic::PROPERTY_INDICATE  // Properti indikasi
    );

    // Menambahkan deskriptor untuk mendukung notifikasi
    pCharacteristic->addDescriptor(new BLE2902());

    // Memulai layanan BLE
    pService->start();

    // Memulai advertising agar klien dapat menemukan server
    advertising();
}

void loop() {
    // Jika ada perangkat yang terhubung
    if (deviceConnected) {
        // Mengirimkan notifikasi dengan nilai terkini
        pCharacteristic->setValue((uint8_t*)&value, 4); // Mengatur nilai yang akan dikirim
        pCharacteristic->notify(); // Mengirimkan notifikasi ke klien
        value++; // Meningkatkan nilai untuk notifikasi berikutnya
        Serial.println("OKE KONEK");
        delay(1000); // Menunggu sebentar untuk menghindari kepadatan stack BLE
    }

    // Jika perangkat telah terputus
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Memberi waktu untuk stack BLE menyelesaikan proses
        pServer->startAdvertising(); // Memulai kembali advertising
        Serial.println("Memulai kembali advertising...");
        oldDeviceConnected = deviceConnected; // Memperbarui status koneksi lama
        Serial.println("OKE PUTUS");
    }

    // Jika perangkat baru saja terhubung
    if (deviceConnected && !oldDeviceConnected) {
        // Anda bisa menambahkan logika lain di sini jika diperlukan
        oldDeviceConnected = deviceConnected; // Memperbarui status koneksi lama
        Serial.println("OKE KONEK LAGI");
    }
}


void advertising(){
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID); // Menambahkan UUID layanan ke iklan
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // Tidak mengiklankan parameter ini
    BLEDevice::startAdvertising(); // Mulai mengiklankan server
    Serial.println("Menunggu koneksi klien untuk notifikasi...");
}
