#include <ESP8266WiFi.h>
#include <VizIoTMqttClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

// Wi-Fi credentials
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// VizIoT credentials
const char* VIZIOT_DEVICE_KEY = "your_16_char_username";
const char* VIZIOT_DEVICE_PASS = "your_20_char_password";

// LED pin
#define LED_PIN D4

// Create MQTT client instance
VizIoTMqttClient mqttClient(VIZIOT_DEVICE_KEY, VIZIOT_DEVICE_PASS);

// LED state
bool ledState = false;

// Timer for sending data to VizIoT MQTT Broker
Ticker sender;
bool isSendDataToServer = false; // Send data only after timer triggers
void SendDataToServer() { isSendDataToServer = true; }
#define INTERVAL_SEND_DATA 300 // Send data every 5 minutes (5*60=300)

// Callback for received parameters
void onParameterReceived(const char* paramName, const char* value) {
  Serial.print("Received parameter: ");
  Serial.print(paramName);
  Serial.print(" = ");
  Serial.println(value);

  if (strcmp(paramName, "led") == 0) {
    bool newState = (strcmp(value, "1") == 0);
    ledState = newState;
    digitalWrite(LED_PIN, ledState ? LOW : HIGH); // Active LOW
    Serial.print("LED state updated to: ");
    Serial.println(ledState ? "ON" : "OFF");
    sendPacketToVizIoT();
  }
}

void sendPacketToVizIoT() {
  DynamicJsonDocument doc(256); // Reduced size for small JSON
  JsonObject obj = doc.to<JsonObject>();
  obj["led"] = ledState ? 1 : 0;
  obj["wifi_rssi"] = WiFi.RSSI();

  String jsonStr;
  serializeJson(doc, jsonStr);

  if (mqttClient.publishJson(jsonStr.c_str())) {
    Serial.println("Published JSON: " + jsonStr);
  } else {
    Serial.println("Failed to publish JSON");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off (active LOW)

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  mqttClient.begin();
  mqttClient.setParameterCallback(onParameterReceived);

  sender.attach(INTERVAL_SEND_DATA, SendDataToServer);
  SendDataToServer(); // Send a data packet as soon as it connects to the broker
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
    return;
  }

  mqttClient.poll();

  if (!mqttClient.isConnected()) {
    mqttClient.reconnect();
  }

  if (isSendDataToServer) {
    isSendDataToServer = false;
    sendPacketToVizIoT();
  }

  delay(100);
}