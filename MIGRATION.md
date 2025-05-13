# Migration Guide: Upgrading from VizIoTMqttClient v1.0.0 to v2.0.0

This guide helps users transition from `VizIoTMqttClient` version 1.0.0 to version 2.0.0. Version 2.0.0 introduces significant improvements, including a switch from `PubSubClient` to `ArduinoMqttClient`, enhanced JSON handling with `ArduinoJson`, broader hardware support, and a more robust API. Below, we detail the changes and provide steps to update your code.

## Key Changes in v2.0.0

1. **Library Dependency Change**:
   - **v1.0.0**: Uses `PubSubClient` for MQTT communication.
   - **v2.0.0**: Uses `ArduinoMqttClient` for improved reliability and compatibility.
   - **Action**: Install `ArduinoMqttClient` and `ArduinoJson` libraries via the Arduino Library Manager.

2. **Hardware Support**:
   - **v1.0.0**: Primarily designed for ESP8266 with limited support for other platforms.
   - **v2.0.0**: Supports multiple platforms (ESP8266, ESP32, WiFiNINA, WiFi101, WiFiC3, WiFiS3) through conditional WiFi includes.
   - **Action**: Ensure the appropriate WiFi library is included for your hardware.

3. **Class Initialization**:
   - **v1.0.0**: Requires passing a `PubSubClient` instance to the constructor.
   - **v2.0.0**: Constructor takes device credentials, host, port, and optional payload size directly; internally manages `MqttClient` and `WiFiClient`.
   - **Action**: Update constructor calls to the new format.

4. **Configuration**:
   - **v1.0.0**: Uses `config()` to set device key, password, host, and port.
   - **v2.0.0**: Configuration is handled in the constructor; `begin()` initializes the MQTT client.
   - **Action**: Move configuration to the constructor and call `begin()` in `setup()`.

5. **Callback Mechanism**:
   - **v1.0.0**: Uses `listenCommand()` with a callback signature `void (*callback)(String, byte)`.
   - **v2.0.0**: Uses `setParameterCallback()` with a callback signature `void (*callback)(const char*, const char*)`.
   - **Action**: Update callback functions to handle `const char*` parameters and use `setParameterCallback()`.

6. **Publishing Data**:
   - **v1.0.0**: `sendJsonString()` sends a JSON string with minimal validation.
   - **v2.0.0**: `publishJson()` validates JSON using `ArduinoJson` before publishing.
   - **Action**: Ensure JSON strings are valid and use `publishJson()`.

7. **Connection Management**:
   - **v1.0.0**: Uses `connect()`, `reconnect()`, `loop()`, and `closeConnection()`.
   - **v2.0.0**: Simplifies to `begin()`, `reconnect()`, `poll()`, and `isConnected()`.
   - **Action**: Replace connection-related calls with the new methods.

8. **Example Sketch**:
   - **v1.0.0**: `mqtt_esp8266_led_control.ino` uses `PubSubClient` and manual WiFi setup.
   - **v2.0.0**: Updated to use `ArduinoMqttClient`, `ArduinoJson`, and streamlined WiFi handling.
   - **Action**: Update your sketch to match the new example structure.

## Migration Steps

### 1. Update Dependencies
- Install the following libraries via the Arduino Library Manager:
  - `ArduinoMqttClient`
  - `ArduinoJson`
- Remove `PubSubClient` if no longer needed.

### 2. Update Library Initialization
In v1.0.0, you initialized the client as follows:

```cpp
WiFiClient espClient;
PubSubClient clientMQTT(espClient);
VizIoTMqttClient clientVizIoT(clientMQTT);
```

In v2.0.0, initialize directly with credentials:

```cpp
VizIoTMqttClient mqttClient(VIZIOT_DEVICE_KEY, VIZIOT_DEVICE_PASS);
```
- Replace `VIZIOT_DEVICE_KEY` and `VIZIOT_DEVICE_PASS` with your 16-character key and 20-character password.

- Optionally specify host, port, and payload size:
  - ```cpp
    VizIoTMqttClient mqttClient(VIZIOT_DEVICE_KEY, VIZIOT_DEVICE_PASS, "viziot.com", 48651, 1024);
    ```
### 3. Update Setup Code
In v1.0.0, configuration and callback setup occurred in `setup()`:

```cpp
clientVizIoT.config(VizIoT_Device_key, VizIoT_Device_pass);
clientVizIoT.listenCommand(callback);
```

In v2.0.0, move configuration to the constructor and call `begin()` and `setParameterCallback()`:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  mqttClient.begin();
  mqttClient.setParameterCallback(onParameterReceived);
}
```

### 4. Update Callback Function
In v1.0.0, the callback used `String` and `byte`:

```cpp
void callback(String parameter, byte value) {
  if (parameter.compareTo("led") == 0) {
    statusLed = (value == 1);
    digitalWrite(LED_ESP, statusLed ? LOW : HIGH);
    snprintf(msg, sizeof(msg), "{\"led\":\"%c\"}", statusLed ? '1' : '0');
    clientVizIoT.sendJsonString(String(msg));
  }
}
```

In v2.0.0, update to use `const char*` and handle JSON publishing:

```cpp
void onParameterReceived(const char* paramName, const char* value) {
  if (strcmp(paramName, "led") == 0) {
    ledState = (strcmp(value, "1") == 0);
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    DynamicJsonDocument doc(256);
    JsonObject obj = doc.to<JsonObject>();
    obj["led"] = ledState ? 1 : 0;
    obj["wifi_rssi"] = WiFi.RSSI();
    String jsonStr;
    serializeJson(doc, jsonStr);
    mqttClient.publishJson(jsonStr.c_str());
  }
}
```

### 5. Update Loop Code
In v1.0.0, the loop managed connection and data sending:

```cpp
void loop() {
  clientVizIoT.loop();
  if (isSendDataToServer) {
    isSendDataToServer = false;
    snprintf(msg, sizeof(msg), "{\"rssi\":\"%i\",\"led\":\"%c\"}", WiFi.RSSI(), statusLed ? '1' : '0');
    clientVizIoT.sendJsonString(String(msg));
  }
}
```

In v2.0.0, use `poll()`, `reconnect()`, and `publishJson()`:

```cpp
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
    DynamicJsonDocument doc(256);
    JsonObject obj = doc.to<JsonObject>();
    obj["led"] = ledState ? 1 : 0;
    obj["wifi_rssi"] = WiFi.RSSI();
    String jsonStr;
    serializeJson(doc, jsonStr);
    mqttClient.publishJson(jsonStr.c_str());
  }
  delay(100);
}
```

### 6. Update Example Sketch
Refer to the updated `mqtt_esp8266_led_control.ino` in the `examples/` folder of v2.0.0. It demonstrates:
- WiFi connection handling
- MQTT client initialization
- JSON data publishing with `ArduinoJson`
- Parameter callback handling
- Periodic data sending with `Ticker`

### 7. Test Your Code
- Upload the updated sketch to your device.
- Monitor the Serial output to verify WiFi and MQTT connections.
- Test LED control and data publishing via the VizIoT platform.

## Example: Updated mqtt_esp8266_led_control.ino
Below is a summary of the key changes in the example sketch:

```cpp
#include <ESP8266WiFi.h>
#include <VizIoTMqttClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* VIZIOT_DEVICE_KEY = "your_16_char_username";
const char* VIZIOT_DEVICE_PASS = "your_20_char_password";
#define LED_PIN D4

VizIoTMqttClient mqttClient(VIZIOT_DEVICE_KEY, VIZIOT_DEVICE_PASS);
bool ledState = false;

Ticker sender;
bool isSendDataToServer = false;
void SendDataToServer() { isSendDataToServer = true; }
#define INTERVAL_SEND_DATA 300

void onParameterReceived(const char* paramName, const char* value) {
  if (strcmp(paramName, "led") == 0) {
    ledState = (strcmp(value, "1") == 0);
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    sendPacketToVizIoT();
  }
}

void sendPacketToVizIoT() {
  DynamicJsonDocument doc(256);
  JsonObject obj = doc.to<JsonObject>();
  obj["led"] = ledState ? 1 : 0;
  obj["wifi_rssi"] = WiFi.RSSI();
  String jsonStr;
  serializeJson(doc, jsonStr);
  mqttClient.publishJson(jsonStr.c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  mqttClient.begin();
  mqttClient.setParameterCallback(onParameterReceived);
  sender.attach(INTERVAL_SEND_DATA, SendDataToServer);
  SendDataToServer();
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

## Additional Notes
- **JSON Validation:** v2.0.0 uses `ArduinoJson` to validate JSON before publishing, reducing errors. Ensure your JSON strings are well-formed.
- **Payload Size:** The constructor allows setting `tx_payload_size` (default: 1024 bytes, min: 128, max: 4096). Adjust if sending large JSON payloads.
- **Error Handling:** v2.0.0 provides better error feedback via Serial output. Monitor the Serial console during development.
- **Platform Compatibility:** Test on your target hardware, as v2.0.0 supports a wider range of boards. Adjust WiFi includes if necessary.

## Support
For issues or questions, please:
- Check the updated example in `examples/mqtt_esp8266_led_control/`.
- Contact support via the VizIoT platform or open an issue on the library’s repository (if available).

By following these steps, you should successfully migrate to `VizIoTMqttClient` v2.0.0 and leverage its enhanced features.
