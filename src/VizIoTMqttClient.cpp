/**
   @file       VizIoTMqttClient.cpp
   @author     VizIoT.com
   @license    This project is released under the MIT License (MIT)
   @copyright  Copyright (c) 2018-2025 VizIoT.com
*/

#include "VizIoTMqttClient.h"

// Initialize static instance pointer
VizIoTMqttClient* VizIoTMqttClient::_instance = nullptr;

// Constructor: Initialize MQTT parameters
VizIoTMqttClient::VizIoTMqttClient(const char* DEVICE_KEY, const char* DEVICE_PASS, const char* mqtt_host, int mqtt_port, unsigned short tx_payload_size) {
  _mqtt_host = mqtt_host;
  _mqtt_port = mqtt_port;
  
  // Limit buffer size
  unsigned short temp_tx_payload_size = tx_payload_size < 4096 ? tx_payload_size : 4096;
  _tx_payload_size = temp_tx_payload_size > 128 ? temp_tx_payload_size : 128;
  
  _DEVICE_KEY = DEVICE_KEY;
  _DEVICE_PASS = DEVICE_PASS;
  _clientId = String("arduinoClient-") + DEVICE_KEY;
  _publishTopic = String("/devices/") + _DEVICE_KEY + "/packet";
  _subscribeTopic = String("/devices/") + _DEVICE_KEY + "/param/+";
  _paramCallback = nullptr;
  _mqttClient = new MqttClient(_wifiClient);
  _instance = this;
}

// Destructor: Clean up MQTT client
VizIoTMqttClient::~VizIoTMqttClient() {
  delete _mqttClient;
  _instance = nullptr;
}

// Initialize MQTT client
void VizIoTMqttClient::begin() {
  _mqttClient->setId(_clientId.c_str());
  _mqttClient->setUsernamePassword(_DEVICE_KEY, _DEVICE_PASS);
  _mqttClient->onMessage(_staticOnMqttMessage);
  _mqttClient->setTxPayloadSize(_tx_payload_size);
  _connectToMqtt();
}

// Connect to MQTT server
void VizIoTMqttClient::_connectToMqtt() {
  if (!_mqttClient->connected()) {
    Serial.println("Connecting to MQTT...");
    if (_mqttClient->connect(_mqtt_host, _mqtt_port)) {
      Serial.println("Connected to MQTT");
      _mqttClient->subscribe(_subscribeTopic);
    } else {
      Serial.print("Failed to connect to MQTT, error: ");
      Serial.println(_mqttClient->connectError());
    }
  }
}

// Static callback to forward to instance method
void VizIoTMqttClient::_staticOnMqttMessage(int messageSize) {
  if (_instance) {
    _instance->_onMqttMessage(messageSize);
  }
}

// Handle incoming MQTT messages
void VizIoTMqttClient::_onMqttMessage(int messageSize) {
  String topic = _mqttClient->messageTopic();
  String paramName = topic.substring(topic.lastIndexOf('/') + 1);
  char payload[256]; // Reduced size for small incoming messages
  int len = _mqttClient->read((unsigned char*)payload, 255);
  payload[len] = '\0';

  if (_paramCallback) {
    _paramCallback(paramName.c_str(), payload);
  }
}

// Publish pre-formed JSON string
bool VizIoTMqttClient::publishJson(const char* jsonString) {
  if (!_mqttClient->connected()) {
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print("Invalid JSON: ");
    Serial.println(error.c_str());
    return false;
  }

  _mqttClient->beginMessage(_publishTopic);
  _mqttClient->print(jsonString);
  return _mqttClient->endMessage();
}

// Set callback for parameter updates
void VizIoTMqttClient::setParameterCallback(void (*callback)(const char* paramName, const char* value)) {
  _paramCallback = callback;
}

// Check if connected to MQTT
bool VizIoTMqttClient::isConnected() {
  return _mqttClient->connected();
}

// Reconnect to MQTT
void VizIoTMqttClient::reconnect() {
  if (!_mqttClient->connected()) {
    _connectToMqtt();
  }
}

// Poll MQTT client
void VizIoTMqttClient::poll() {
  _mqttClient->poll();
}