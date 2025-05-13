# VizIoTMqttClient

An Arduino library for connecting to the VizIoT MQTT broker, publishing large JSON data, and subscribing to parameter updates. This library uses `ArduinoMqttClient` for asynchronous MQTT communication, validates pre-formed JSON, and provides parameter values as strings for flexible user handling. It supports multiple platforms, including ESP8266, ESP32, SAMD, and more.

## Features
- Connects to the VizIoT MQTT broker (default: `viziot.com:48651`).
- Publishes pre-formed JSON strings to `/devices/DEVICE_KEY/packet` (up to 1024 bytes by default).
- Subscribes to `/devices/DEVICE_KEY/param/+` for parameter updates.
- Validates JSON before publishing with a 1024-byte buffer.
- Provides parameter values as strings via a callback, allowing user-defined type conversion.
- Supports ESP8266, ESP32, SAMD, Portenta, and other Arduino platforms.
- Configurable MQTT host, port, and transmit payload size.
- User-managed Wi-Fi connectivity.

## Installation

### Using Arduino IDE
1. Open the Arduino IDE Library Manager (`Sketch > Include Library > Manage Libraries`).
2. Search for `VizIoTMqttClient` and click `Install`.
3. Install dependencies:
   - `ArduinoMqttClient` by Arduino
   - `ArduinoJson` by Benoit Blanchon

### Manual Installation
1. Clone or download this repository: `git clone https://github.com/VizIoT-com/viziot-mqtt-client-arduino.git`.
2. Place the `VizIoTMqttClient` folder in your Arduino `libraries` directory (e.g., `~/Arduino/libraries/`).
3. Install dependencies via Library Manager or their respective repositories.

### Platform Support
- ESP8266 (e.g., NodeMCU, Wemos D1 Mini)
- ESP32
- SAMD (e.g., MKR1000, Nano 33 IoT)
- Portenta H7, Nicla Vision, Giga, Opta
- Other Arduino boards with WiFi support
- Requires the appropriate board core (e.g., `http://arduino.esp8266.com/stable/package_esp8266com_index.json` for ESP8266, installed via Board Manager).

## Usage

### Example: LED Control
The example (`examples/mqtt_esp8266_led_control/mqtt_esp8266_led_control.ino`) demonstrates controlling an LED on pin D4 (ESP8266), publishing its state and Wi-Fi RSSI every 5 minutes using a `Ticker`, and subscribing to parameter updates to control the LED.

```cpp
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
```

## Connection Parameters
- **Server:** Configurable, defaults to `viziot.com`
- **Port:** Configurable, defaults to `48651`
- **Device Key:** 16-character VizIoT device key
- **Device Password:** 20-character VizIoT device password
- **Client ID:** Auto-generated as arduinoClient-DEVICE_KEY
- **Publish Topic:** `/devices/DEVICE_KEY/packet`
- **Subscribe Topic:** `/devices/DEVICE_KEY/param/+`

## Methods
- `begin()`: Initializes the MQTT client.
- `publishJson(const char* jsonString)`: Publishes a validated JSON string.
- `setParameterCallback(void (*callback)(const char* paramName, const char* value))`: Sets a callback for parameter updates.
- `isConnected()`: Returns `true` if connected to the MQTT broker.
- `reconnect()`: Attempts to reconnect to the MQTT broker.
- `poll()`: Processes MQTT events (call regularly).

## Testing
- Update the example with your Wi-Fi and VizIoT credentials.
- Connect an LED to pin D4 (ESP8266; anode to D4, cathode to GND via a resistor; active LOW).
- Upload the sketch to an ESP8266 board.
- Monitor output via Serial Monitor (115200 baud).
- Test LED control by publishing to `/devices/DEVICE_KEY/param/led` (e.g., `"1"`, `"0"`) using an MQTT client like MQTTX.
- Verify JSON publications every 5 minutes to `/devices/DEVICE_KEY/packet` (e.g., `{"led":1,"wifi_rssi":-65}`).

## Notes
- Ensure Wi-Fi is connected before calling `begin()` or `reconnect()`.
- Invalid JSON strings are rejected by `publishJson`.
- Parameter values are provided as strings; convert them as needed (e.g., `atof(value)` for floats).
- Adjust the payload buffer size in `VizIoTMqttClient.cpp` if larger incoming messages are expected.
- TLS support may require replacing `WiFiClient` with `WiFiSSLClient`.

## License
MIT License. See LICENSE (LICENSE.txt) for details.

## Support
For issues or feature requests, open an issue on GitHub.

