# Bus protocol

Everything below was observed by listening on the RS485 connector `CN5` of an Aquathermica Pro 260 (see [hardware.md](hardware.md)). I do not know which board on the bus is the sender of these frames; the project only listens.

## Serial parameters

**9600 baud, 8 data bits, no parity, 1 stop bit (8N1).**

## Frames

The bus carries plain **Modbus-RTU** frames with **function code 0x10 (Write Multiple Registers)** and unit address `0x63`:

```
63 10 | start addr (2) | quantity (2) | byte count (1) | quantity × 2 data bytes (big endian) | CRC16 lo hi
```

Three blocks are sent in turn, one frame about every 2.7 s:

| Start address | Quantity | Frame length |
|---|---|---|
| 1001 (`03 E9`) | 90 (`00 5A`) | 189 bytes |
| 1091 (`04 43`) | 90 | 189 bytes |
| 2001 (`07 D1`) | 90 | 189 bytes |

Example header of the first block: `63 10 03 E9 00 5A B4 …` (byte count `B4` = 180 = 90 × 2). Three complete sample frames are in [`tests/frames/`](../tests/frames/).

Registers 1001… hold settings/parameters (setpoint, mode, defrost/disinfection/fan parameters, clock); registers 2001… hold live measurements and output states. See [registers.md](registers.md).

## Gateway view (archived Node-RED path)

A Waveshare RS485-to-Ethernet gateway (set up for Modbus RTU↔TCP conversion) strips the CRC and prepends an MBAP header (`00 01 00 00 00 BB`, length `0xBB` = 187):

```
00 01 00 00 00 BB | 63 10 03 E9 00 5A B4 | … data …
```

The Node-RED flow parses MBAP → FC 0x10 → start address → quantity → byte count (must equal quantity × 2) → registers.

## Decoding strategy (both implementations)

* Resync byte by byte on anything that is not a consistent frame.
* Store each register under its **address** from the header, never by byte offset – this is what makes the decoder robust if the manufacturer inserts or removes registers.
* ESPHome additionally verifies the Modbus CRC16 (polynomial 0xA001); the gateway has already stripped it for Node-RED.
* Values older than 180 s become *unknown/unavailable* (`EXPIRE_S` in the flow, `STALE_MS` in the decoder).

## Pitfall: stop bits

The unit transmits **1 stop bit**. If the receiver is configured for **2 stop bits** it samples the stop-bit period too long and loses synchronisation; bytes – mostly `0x00` – vanish and the stream looks "compressed", with register positions drifting and varying frame lengths. My first ESP configuration used 2 stop bits, which was simply wrong. I reproduced it by simulating an 8N2 UART receiving an 8N1 stream; it yields exactly the garbage I saw. With `stop_bits: 1` all frames pass the CRC.

If you see `CRC error` messages or "RS485 CRC errors" counting up, check this first.

## Not decoded / unknown

* Registers outside 1000–1199 and 2000–2199 are ignored by the ESPHome decoder (storage windows).
* Only FC 0x10 frames were identified in my captures; the project ignores any other traffic.
* Writing to the bus is not implemented.
