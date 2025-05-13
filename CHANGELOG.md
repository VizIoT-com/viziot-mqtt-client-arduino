# VizIoT MQTT Client Changelog

## [2.0.0] - 2025-05-13
### Added
- Support for large JSON messages (up to 1024 bytes by default) via `ArduinoMqttClient`’s `setTxPayloadSize`.
- Multi-platform WiFi support for ESP8266, ESP32, SAMD, Portenta, and other Arduino boards.
- Configurable MQTT host and port in constructor.
- Destructor to clean up MQTT client resources.
- Static callback for `ArduinoMqttClient`’s `onMessage` compatibility.
- Validation of JSON payloads before publishing.

### Changed
- Switched from `PubSubClient` to `ArduinoMqttClient` for asynchronous MQTT operations and larger payload support.
- Updated `publishJson` to validate JSON with a 1024-byte buffer.
- Reduced incoming payload buffer to 256 bytes for efficiency.
- Updated example to use `Ticker` for precise 5-minute intervals and smaller JSON buffer (256 bytes).
- Updated dependencies to `ArduinoMqttClient` and `ArduinoJson`.

### Removed
- Dependency on `PubSubClient`.

## [1.0.1] - 2024-03-19
### Changed
- Fixed bugs and updated the example.

## [1.0.0] - 2018-11-28
### Added
- Initial release using `PubSubClient` for MQTT communication.
- Support for sending JSON data and receiving commands.
- Tested on ESP8266.