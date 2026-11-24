# Node-RED path (ARCHIVED – first attempt, no longer developed)

`flow.json` reads the Modbus-TCP stream of a Waveshare RS485-to-Ethernet gateway, decodes it and publishes **MQTT discovery** configs plus a state topic, so Home Assistant creates the entities automatically.

## Requirements

* Waveshare *RS485 to RJ45 Ethernet Industrial Serial Server Modbus Gateway* (or any gateway that delivers **Modbus-TCP frames** of the passively received RTU traffic)
* Node-RED, MQTT broker (e.g. Mosquitto), Home Assistant MQTT integration

## Gateway settings

The flow expects what the gateway produced in the author's setup:

| Setting | Value |
|---|---|
| Device port | 502 |
| Work mode | **TCP Server** (Node-RED connects as TCP *client*) |
| Serial | Baud rate **9600**, Databits **8**, Parity **None**, Stopbits **1**, Flow control None |
| Multi-Host Settings → Protocol | **Modbus TCP to RTU** (output frames start with an MBAP header such as `00 01 00 00 00 BB 63 10 …`) |
| Multi-Host Settings → Enable Multi-host | No (Instruction Time out 0, RS485 Conflict Time Gap 20 ms) |
| Advanced | No-Data-Restart: Disable |
| Wiring | A/B in parallel to CN5 A/B, see [../../docs/hardware.md](../docs/hardware.md) |

Screenshot of the author's working gateway configuration (firmware V1.523; network addresses and MAC blacked out): [waveshare-settings.png](../../docs/images/waveshare-settings.png). Destination IP/port are irrelevant in TCP Server mode.

## Import

1. Node-RED → ☰ → Import → `flow.json`.
2. Open the node **Waveshare RS485 gateway** and set the gateway IP (placeholder `192.168.1.50`), port 502.
3. Open the **mosquitto** broker config node and set your broker (placeholder `homeassistant.local`) and credentials.
4. Deploy. After ~5 s the discovery configs are published; whenever Home Assistant announces `online` they are re-sent.

State is published to `tesy/bwwp/state` once all three blocks have been received, on change, and every 60 s as heartbeat. Entities expire (become *unavailable*) after 180 s without data.

Entity IDs: `sensor.tesy_bwwp_<key>` / `binary_sensor.tesy_bwwp_<key>` (e.g. `sensor.tesy_bwwp_t2020` = reg 2020, bottom of tank). The `SENSORS` list at the top of the function node defines everything (register, scaling, name, enabled by default).

## Notes

* The flow is address-based: it stores `regs[address]` from every FC 0x10 frame.
* The "debug: raw TCP stream" node is disabled by default.
* Compared with the first version of the author's flow, the "HA restart" branch is now wired to the decoder node so that discovery is re-sent when HA restarts.
