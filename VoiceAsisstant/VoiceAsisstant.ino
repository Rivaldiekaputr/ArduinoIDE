// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

#define WIFI_SSID         "Rumahku"
#define WIFI_PASS         "PodoWingi"
#define APP_KEY           "3a014187-b41b-4c6d-a8c0-43cc8ea0ff99"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "be0c203a-0569-4d9b-a9eb-cd422d450e3e-bf8f4fff-a00f-40c4-aeb7-4d6ad8c08546"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

// Enter the device IDs here
#define device_ID_1   "674f11b313f98f1416fd5833"
#define device_ID_2   "674f11cb039b4bdc5d3306b0"

// Define the GPIO connected with Relays
#define RelayPin1 5  //D1
#define RelayPin2 4 //D2
#define BAUD_RATE   9600

typedef struct {
  int relayPIN;
} deviceConfig_t;

// Main configuration for devices
std::map<String, deviceConfig_t> devices = {
  {device_ID_1, { RelayPin1 }},
  {device_ID_2, { RelayPin2 }}
};

void setupRelays() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, HIGH); // Pastikan relay OFF saat memulai
    delay(100); // Beri waktu stabilisasi
  }
}

bool onPowerState(String deviceId, bool &state)
{
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN;
  digitalWrite(relayPIN, !state); // Relay active LOW
  return true;
}

void setupWiFi()
{
  Serial.printf("\r\n[WiFi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro()
{
  for (auto &device : devices)
  {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setup() {
  Serial.begin(BAUD_RATE);
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, HIGH); // Default kondisi mati sebelum memulai
  }
  setupRelays();
  setupWiFi();
  setupSinricPro();
}


void loop()
{
  SinricPro.handle();
}
