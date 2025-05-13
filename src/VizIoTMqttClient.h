/**
   @file       VizIoTMqttClient.h
   @author     VizIoT.com
   @license    This project is released under the MIT License (MIT)
   @copyright  Copyright (c) 2018-2025 VizIoT.com
*/

#ifndef VIZIOT_MQTT_CLIENT_H
#define VIZIOT_MQTT_CLIENT_H

#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA) || defined(ARDUINO_OPTA)
  #include <WiFi.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <WiFiC3.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#endif

class VizIoTMqttClient {
public:
  /**
   * @brief Constructor for VizIoTMqttClient.
   * @param DEVICE_KEY 16-character VizIoT device key.
   * @param DEVICE_PASS 20-character VizIoT device password.
   * @param mqtt_host MQTT server hostname (default: "viziot.com").
   * @param mqtt_port MQTT server port (default: 48651).
   * @param tx_payload_size Transmit payload buffer size in bytes (default: 1024).
   */
  VizIoTMqttClient(const char* DEVICE_KEY, const char* DEVICE_PASS, const char* mqtt_host = "viziot.com", int mqtt_port = 48651, unsigned short tx_payload_size = 1024);

  /**
   * @brief Destructor to clean up resources.
   */
  ~VizIoTMqttClient();

  /**
   * @brief Initializes the MQTT client and connects to the broker.
   */
  void begin();

  /**
   * @brief Publishes a pre-formed JSON string to the publish topic.
   * @param jsonString The JSON string to publish.
   * @return true if successful, false otherwise.
   */
  bool publishJson(const char* jsonString);

  /**
   * @brief Sets the callback for received parameter updates.
   * @param callback Function to call with paramName and value.
   */
  void setParameterCallback(void (*callback)(const char* paramName, const char* value));

  /**
   * @brief Checks if the client is connected to the MQTT broker.
   * @return true if connected, false otherwise.
   */
  bool isConnected();

  /**
   * @brief Attempts to reconnect to the MQTT broker if disconnected.
   */
  void reconnect();

  /**
   * @brief Polls the MQTT client to process incoming messages.
   */
  void poll();

private:
  const char* _mqtt_host;
  int _mqtt_port;
  unsigned short _tx_payload_size;
  const char* _DEVICE_KEY;
  const char* _DEVICE_PASS;
  String _clientId;
  MqttClient* _mqttClient;
  WiFiClient _wifiClient;
  String _publishTopic;
  String _subscribeTopic;
  void (*_paramCallback)(const char* paramName, const char* value);
  static VizIoTMqttClient* _instance;

  void _connectToMqtt();
  void _onMqttMessage(int messageSize);
  static void _staticOnMqttMessage(int messageSize);
};

#endif