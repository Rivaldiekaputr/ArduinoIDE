#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <ESP8266Wifi.h>

const char DEVICE_LOGIN_NAME[]  = "12c10f02-fb3f-4ca7-9e3f-6c6f662b1b9b"; //Enter DEVICE ID
const char SSID[]               = "Rumahku";    //Enter WiFi SSID (name)
const char PASS[]               = "PodoWingi";    //Enter WiFi password
const char DEVICE_KEY[]         = "SMGse!!E@9X5x!yF8pYWQnn22";    //Enter Secret device password (Secret Key)

#define RelayPin1 5  //D1
#define RelayPin2 4  //D2

int toggleState_1 = 0; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 0; //Define integer to remember the toggle state for relay 2

void onSwitch1Change();
void onSwitch2Change();

CloudSwitch switch1;
CloudSwitch switch2;

void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  ArduinoCloud.addProperty(switch1, READWRITE, ON_CHANGE, onSwitch1Change);
  ArduinoCloud.addProperty(switch2, READWRITE, ON_CHANGE, onSwitch2Change);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

unsigned long previousMillis = 0;
const long interval = 500; // interval at which to blink (milliseconds)
int wifiStatus = 0; // 0: Connecting, 1: Connected

void setup() {
  Serial.begin(9600);
  delay(1500); // Delay to wait for serial monitor

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);

  // During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
}

void loop() {
  ArduinoCloud.update();

  // WiFi Connection Status
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiStatus == 0) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        Serial.print("."); // Show dot while trying to connect
      }
    }
  } else {
    if (wifiStatus == 0) {
      Serial.println("\nWiFi Connected!");
      wifiStatus = 1; // Set the flag when WiFi is connected
    }
  }
}

void onSwitch1Change() {
  if (switch1 == 1) {
    digitalWrite(RelayPin1, LOW);
    Serial.println("Device1 ON");
    toggleState_1 = 1;
  } else {
    digitalWrite(RelayPin1, HIGH);
    Serial.println("Device1 OFF");
    toggleState_1 = 0;
  }
}

void onSwitch2Change() {
  if (switch2 == 1) {
    digitalWrite(RelayPin2, LOW);
    Serial.println("Device2 ON");
    toggleState_2 = 1;
  } else {
    digitalWrite(RelayPin2, HIGH);
    Serial.println("Device2 OFF");
    toggleState_2 = 0;
  }
}
