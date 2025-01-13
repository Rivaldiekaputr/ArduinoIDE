#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

// Nama perangkat yang diinginkan
#define DEVICE_NAME "ESP32_DHT11_Server"

// UUID untuk layanan dan karakteristik BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define TEMP_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a7"
#define HUMID_CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Variabel untuk menyimpan data dari karakteristik
float temperature = 0.0;
float humidity = 0.0;

BLEAdvertisedDevice* myDevice = nullptr; // Perangkat yang diiklankan

// Callback untuk menerima notifikasi dari server
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
        Serial.println("Terhubung ke server BLE!");
    }

    void onDisconnect(BLEClient* pClient) {
        Serial.println("Terputus dari server BLE.");
    }
};

// Callback untuk perangkat yang diiklankan selama scanning
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.print("Ditemukan perangkat: ");
        Serial.println(advertisedDevice.toString().c_str());

        // Cek apakah nama perangkat cocok dengan yang dicari
        if (advertisedDevice.getName() == DEVICE_NAME) {
            Serial.println("Perangkat ditemukan dengan nama yang sesuai, mencoba menghubungkan...");
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            BLEDevice::getScan()->stop(); // Hentikan scanning
        }
    }
};

bool connectToServer() {
    Serial.println("Mencoba terhubung ke server BLE...");

    BLEClient* pClient = BLEDevice::createClient(); // Membuat client
    pClient->setClientCallbacks(new MyClientCallback()); // Set callback setelah membuat client

    if (!pClient->connect(myDevice)) {
        Serial.println("Gagal terhubung ke server.");
        return false;
    }
    Serial.println("Terhubung ke server!");

    // Mengakses layanan BLE
    BLERemoteService* pService = pClient->getService(BLEUUID(SERVICE_UUID));
    if (!pService) {
        Serial.println("Layanan tidak ditemukan.");
        pClient->disconnect();
        return false;
    }
    Serial.println("Layanan BLE berhasil terhubung.");

    // Mengakses karakteristik suhu
    BLERemoteCharacteristic* pTempCharacteristic = pService->getCharacteristic(BLEUUID(TEMP_CHAR_UUID));
    if (!pTempCharacteristic) {
        Serial.println("Karakteristik suhu tidak ditemukan.");
        pClient->disconnect();
        return false;
    }
    Serial.println("Karakteristik suhu berhasil terhubung.");

    // Mengakses karakteristik kelembaban
    BLERemoteCharacteristic* pHumidCharacteristic = pService->getCharacteristic(BLEUUID(HUMID_CHAR_UUID));
    if (!pHumidCharacteristic) {
        Serial.println("Karakteristik kelembaban tidak ditemukan.");
        pClient->disconnect();
        return false;
    }
    Serial.println("Karakteristik kelembaban berhasil terhubung.");

    // Mendaftar untuk menerima notifikasi dari karakteristik suhu
    if (pTempCharacteristic->canNotify()) {
        pTempCharacteristic->registerForNotify([](BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
            Serial.print("Notifikasi suhu diterima");

            if (length == sizeof(float)) {
                float temp = *((float*)pData);
                Serial.print("Suhu: ");
                Serial.println(temp);
            } else {
                Serial.println("Data suhu tidak valid.");
            }
        });
    }

    // Mendaftar untuk menerima notifikasi dari karakteristik kelembaban
    if (pHumidCharacteristic->canNotify()) {
        pHumidCharacteristic->registerForNotify([](BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
            Serial.print("Notifikasi kelembaban diterima);

            if (length == sizeof(float)) {
                float hum = *((float*)pData);
                Serial.print("Kelembaban: ");
                Serial.println(hum);
            } else {
                Serial.println("Data kelembaban tidak valid.");
            }
        });
    }

    return true;
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init(""); // Inisialisasi BLE

    // Mulai scanning untuk menemukan server
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false); // Durasi scanning 5 detik
}

void loop() {
    if (myDevice != nullptr) {
        if (connectToServer()) {
            Serial.println("Koneksi berhasil, menerima notifikasi...");
        } else {
            Serial.println("Koneksi gagal, mencoba lagi...");
        }
        myDevice = nullptr; // Reset perangkat setelah mencoba koneksi
    }

    delay(1000); // Delay untuk mencegah pengulangan terlalu cepat
}
