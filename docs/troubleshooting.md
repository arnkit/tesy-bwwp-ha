# Troubleshooting

## ESPHome

| Symptom | Likely cause | What to do |
|---|---|---|
| **RS485 bytes received** = 0 | Nothing reaches GPIO13 | Check A/B, VCC/GND, module pin "TX" → D7, module "RX" left open. The module's RX LED should blink about every 2.7 s. Try swapping A and B. |
| Bytes count up, **valid frames** = 0 | Wrong serial parameters | Must be 9600 baud, parity NONE, **stop bits 1**. |
| **CRC errors** keep rising | Stop bits 2, noise, bad A/B contact | Check `stop_bits: "1"`; shorten/twist wires; connect GND. A few isolated errors are not a concern. |
| Entities `unknown` | No valid frame for > 180 s, or block not yet received | Wait about 10 s after boot (all three blocks must arrive once); check the "RS485 age of last frame" sensor. |
| Everything works, then ESP reboots | Out of RAM | Enable fewer extra entities. |
| Compile error about `bit` | Naming clash with Arduino's `bit()` macro | Use the decoder from this repo (method is called `bit_set`). |
| Values look shifted / garbage | Old decoder versions or 2 stop bits | Use `tesy_decoder.h` v3 with `stop_bits: 1`. |

Enable raw logging by un-commenting the `debug:` block under `uart:` and setting `logger: level: DEBUG`.

## Node-RED (archived)

| Symptom | Likely cause |
|---|---|
| No entities in HA | MQTT integration/broker not configured; check the broker node and the "Discovery on start" inject (also triggered when HA announces `online`). |
| Entities `unavailable` | No frames for 180 s: check the gateway connection (TCP client to port 502 of the gateway). |
| Nothing decoded | The gateway must deliver Modbus-TCP (MBAP) frames; see [archive/node-red-waveshare/README.md](../archive/node-red-waveshare/README.md). |
| Duplicate entities | Both the ESPHome device and the Node-RED flow are active against the same HA. Run only one. |
