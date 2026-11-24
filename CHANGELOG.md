# Changelog

## 0.1.0 – initial public release
* ESPHome decoder v3: Modbus-RTU (FC 0x10), CRC16, address-based register storage, byte-wise resync.
* ESPHome YAML for Wemos D1 mini (UART0 swap, RX only on GPIO13/D7), 9600 8N1.
* Optional package with ~90 extra entities (disabled by default).
* Node-RED flow (archived, first attempt): Waveshare Modbus-TCP stream → decode → MQTT discovery (110 entities).
* Example Home Assistant dashboard.
* Host tests with captured frames.
