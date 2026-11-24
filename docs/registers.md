# Register map

All values are 16-bit registers carried by Modbus FC 0x10 frames on the RS485 bus (see [protocol.md](protocol.md)). Registers are addressed as they appear in the frame (1001…1180, 1091…, 2001…).

**The `extra` registers are only partly verified** (see below).

**Reliability legend.** The register ↔ parameter assignment is the author's own finding (comparing values with the unit's display and the TESY *Installation & Maintenance* manual). Entries with `(?)`, `(unclear)` or `(undocumented)` in their names are hypotheses.

* **Parameter**: code printed in the TESY manual (`d`efrost, `g` disinfection, `E` expansion valve, `n` solar/circulation, `H` system, `r` temperatures/heater, `F` fan, `t` measured temperatures, `O` outputs).
* **Manual default**: default value as printed in the manual (physical units, *not* raw). A device may be configured differently.
* **Main**: `yes` = in `esphome/tesy-bwwp.yaml` and enabled by default in the archived Node-RED flow; `extra` = in `esphome/tesy-bwwp-extra.yaml` (disabled by default).

## Decoding rules

| Type | Rule |
|---|---|
| temperature | `T[°C] = raw / 2 − 30` |
| raw | value as is (some are divided: fan speed reg 2067 ÷ 10, superheat reg 2066 ÷ 10) |
| bool | `raw ≠ 0` |
| bit | `(raw >> n) & 1` |
| duration (days / min) | reg 2061 raw = days; reg 1109 raw × 5 = minutes |

### Operating mode (reg 1012, with reg 1011)

| reg 1011 | reg 1012 | Mode |
|---|---|---|
| 0 | any | Off |
| ≠ 0 | 0 | Smart |
| ≠ 0 | 2 | ECO |
| ≠ 0 | 3 | Auto |
| ≠ 0 | 4 | High demand |
| ≠ 0 | 7 | Vacation |
| ≠ 0 | 8 | Boost |

The TESY manual calls "Smart" *Intelligent mode* and "Vacation" *vacation mode*.

### Output status bits (reg 2050)

| Bit | Code | Meaning |
|---|---|---|
| 8 | O08 | Compressor |
| 9 | O09 | Electric heater |
| 10 | O10 | Four-way / two-way valve |
| 11 | O11 | Solar circulating pump |
| 12 | O12 | Solar drain valve |
| 13 | O13 | Fan high speed |
| 14 | O14 | Fan low speed |

### Device clock

Reg 1152 minute · 1153 hour · 1154 day · 1155 month · 1156 year (2 digits).

## Table

| Register | Parameter | Meaning | Decoding | Manual default | Main |
|---|---|---|---|---|---|
| 1011 |  | Device on (0 = off) | raw ≠ 0 |  | yes |
| 1012 |  | Operating mode | mode map (see below) |  | yes |
| 1013 |  | (operating mode?) 2 -> 8 on Boost | raw |  | extra |
| 1023 |  | (undocumented, temp?) -15 | T = raw/2 − 30 °C |  | extra |
| 1024 |  | (undocumented, temp?) 15 | T = raw/2 − 30 °C |  | extra |
| 1026 |  | (undocumented, temp?) -4 | T = raw/2 − 30 °C |  | extra |
| 1029 |  | (undocumented, temp?) -5 | T = raw/2 − 30 °C |  | extra |
| 1030 |  | (undocumented, temp?) 0 | T = raw/2 − 30 °C |  | extra |
| 1033 |  | (factory date? 0x0819 = 08/25) | raw |  | extra |
| 1034 | d04 | Defrost entry temperature | T = raw/2 − 30 °C | -7 °C | extra |
| 1035 | d03 | Defrost exit temperature | T = raw/2 − 30 °C | 13 °C | extra |
| 1036 | d01 | Defrost cycle [min] | raw | 45 min | extra |
| 1037 | d02 | Maximum defrosting time [min] | raw | 8 min | extra |
| 1038 | d06 | Defrost ambient temperature deviation after slippage | T = raw/2 − 30 °C | 14 °C | extra |
| 1040 | d08 | Defrost coil temperature deviation after slippage (2) | T = raw/2 − 30 °C | 2 °C | extra |
| 1041 | d07 | Defrost coil temperature deviation after slippage (10) | T = raw/2 − 30 °C | 10 °C | extra |
| 1042 | d10 | Slip end coil temperature (?) | T = raw/2 − 30 °C |  | extra |
| 1043 | d11 | Eco defrost minimum time [min] | raw |  | extra |
| 1046 | g01 | Disinfection target temperature | T = raw/2 − 30 °C | 63 °C | extra |
| 1047 |  | Legionella protection | raw ≠ 0 |  | yes |
| 1047 | g02 | Disinfection maintenance time [min] | raw | 40 min | extra |
| 1048 | g03 | Disinfection start time [h] | raw | 23 h | extra |
| 1049 | g04 | Disinfection cycle [days] | raw | 30 days | extra |
| 1051 | d05 | Slip defrosting coil temperature minimum | T = raw/2 − 30 °C | -18 °C | extra |
| 1055 | E01 | EEV adjustment (1 = automatic) | raw | 1 | extra |
| 1056 | E02 | EEV target superheat | T = raw/2 − 30 °C | 5 °C | extra |
| 1057 | E03 | EEV initial steps (raw = steps / 10) | raw | 240 | extra |
| 1058 | E04 | EEV minimum steps (raw = steps / 10) | raw | 100 | extra |
| 1059 | E05 | EEV defrost steps (raw = steps / 10) | raw | 480 | extra |
| 1062 | n13 | Circulation pump start temperature | T = raw/2 − 30 °C | 38 °C | extra |
| 1063 | n14 | Circulation pump start temperature difference | T = raw/2 − 30 °C | 10 °C | extra |
| 1066 | H05 | Time after thermostat shutdown in economy mode [min] | raw | 5 | extra |
| 1067 | H01 | Power-down memory function | raw | 1 | extra |
| 1068 | H03 | Mixed water / lower water temperature proportion | raw | 10 | extra |
| 1074 |  | (undocumented) 3 | raw |  | extra |
| 1076 | H08 | H08? - Central control address (?) | raw |  | extra |
| 1077 |  | (undocumented) 1 | raw |  | extra |
| 1081 | n02 | Solar pump maximum running time [min] | raw | 15 min | extra |
| 1082 | n03 | Solar pump start-up temperature difference | T = raw/2 − 30 °C | 20 °C | extra |
| 1085 | n06 | Night cooling stop [h] | raw |  | extra |
| 1086 | n07 | Night cooling start temperature | T = raw/2 − 30 °C |  | extra |
| 1087 | n08 | Night cooling stop differential | T = raw/2 − 30 °C |  | extra |
| 1088 | n09 | Solar drain valve temperature | T = raw/2 − 30 °C |  | extra |
| 1089 | n10 | Solar pump stop temperature | T = raw/2 − 30 °C | 50 °C | extra |
| 1104 |  | Setpoint temperature | T = raw/2 − 30 °C |  | yes |
| 1105 | r02 | Enable switch solar / low pressure | raw |  | extra |
| 1106 | r03 | Restart temperature difference (T02) | T = raw/2 − 30 °C | 5 °C | extra |
| 1107 | r04 | Electric heating setting temperature enabled (= boost flag?) | raw | 0 | extra |
| 1107 | r04 | Electric heating enabled (Boost or High-Demand) | raw ≠ 0 | 0 | yes |
| 1108 | r05 | Electric heating setting temperature | T = raw/2 − 30 °C | 55 °C | extra |
| 1109 | r06 | Electric heating start delay (raw = min / 5) | raw × 5 min | 200 min | extra |
| 1110 | r07 | Electric heating replaces compressor | raw | 1 | extra |
| 1111 | r08 | Ambient temperature: electric heating replaces compressor | T = raw/2 − 30 °C | -7 °C | extra |
| 1112 | r09 | Ambient temperature: electric heating starts without delay | T = raw/2 − 30 °C | 5 °C | extra |
| 1113 | r10 | Ambient temperature: electric heating delayed start | T = raw/2 − 30 °C | 25 °C | extra |
| 1115 | r12 | Compressor forced-stop temperature | T = raw/2 − 30 °C |  | extra |
| 1116 | r13 | External control (PV / SG) | raw | 5 | extra |
| 1117 | r14 | Second external temperature set point (PV) | T = raw/2 − 30 °C | 75 °C | extra |
| 1118 | r15 | High-temperature compressor stop temperature | T = raw/2 − 30 °C | 78 °C | extra |
| 1119 | r16 | r16? - (undocumented) value 3 | raw |  | extra |
| 1120 | r17 | Restart based on TTT | raw ≠ 0 |  | extra |
| 1121 | r18 | Restart temperature difference at top of tank | T = raw/2 − 30 °C | 3 °C | extra |
| 1122 | r19 | Compressor stop temperature 1 | T = raw/2 − 30 °C | 65 °C | extra |
| 1123 | r20 | Compressor stop temperature 2 | T = raw/2 − 30 °C | 55 °C | extra |
| 1124 | r21 | Highest ambient temperature: electric heating replaces compressor | T = raw/2 − 30 °C | 43 °C | extra |
| 1125 | r22 | Restart temperature difference (T03, High-Demand mode) | T = raw/2 − 30 °C | 5 °C | extra |
| 1129 |  | Vacation active | raw ≠ 0 |  | yes |
| 1130 |  | (vacation parameter?) 24 | raw |  | extra |
| 1131 |  | (vacation parameter?) 1 | raw |  | extra |
| 1132 |  | (vacation parameter?) 1 | raw |  | extra |
| 1143 |  | No vacation (0 on vacation change) | raw ≠ 0 |  | extra |
| 1162 | F03 | Fan fault logic enable | raw | 1 | extra |
| 1163 | F04 | Fan high speed value | raw | 110 | extra |
| 1164 | F05 | Fan low speed value | raw | 30 | extra |
| 1165 | F06 | Fan coil temperature at high point | T = raw/2 − 30 °C | 15 | extra |
| 1166 | F07 | Fan coil temperature at low point | T = raw/2 − 30 °C | 35 | extra |
| 1168 | F09 | DC fan speed 1 | raw |  | extra |
| 1169 | F10 | DC fan speed 2 | raw |  | extra |
| 1170 | F11 | DC fan speed 3 | raw |  | extra |
| 1171 | F12 | DC fan speed 4 | raw |  | extra |
| 1172 | F13 | DC fan speed 5 | raw | 83 | extra |
| 1173 |  | (fan parameter?) 30 | raw |  | extra |
| 1174 |  | (fan parameter?) 910 | raw |  | extra |
| 2011 |  | (unclear) 32 | raw |  | extra |
| 2012 | O17 | DSP software version (?) | raw |  | extra |
| 2013 |  | (unclear) 30, copy of reg 1173 | raw |  | extra |
| 2014 | O18 | PFC software version (?) | raw |  | extra |
| 2015 |  | Device on (mirror of reg 1011) | raw ≠ 0 |  | extra |
| 2016 |  | Mode status word (= reg 1012 x 16) | raw |  | extra |
| 2019 | t01 | AT ambient temperature | T = raw/2 − 30 °C |  | yes |
| 2020 | t02 | BTT bottom of tank | T = raw/2 − 30 °C |  | yes |
| 2021 | t03 | TTT top of tank | T = raw/2 − 30 °C |  | yes |
| 2022 | t04 | CT coil (evaporator) temperature | T = raw/2 − 30 °C |  | yes |
| 2023 | t05 | SUT suction temperature (compressor refrigerant inlet) | T = raw/2 − 30 °C |  | yes |
| 2024 | t06 | Solar / recirculation water sensor (?) | T = raw/2 − 30 °C |  | extra |
| 2025 |  | Actual temperature | T = raw/2 − 30 °C |  | yes |
| 2028 | t07 | ET exhaust temperature (compressor outlet) | T = raw/2 − 30 °C |  | yes |
| 2030 |  | (unclear) 39 compressor on / 40 off / 69 boost | raw |  | extra |
| 2050.08 | O08 | Compressor (bit 8) | bit 8 |  | yes |
| 2050.09 | O09 | Electric heater (bit 9) | bit 9 |  | yes |
| 2050.10 | O10 | Four-way or two-way valve (bit 10) | bit 10 |  | extra |
| 2050.11 | O11 | Solar circulating pump (bit 11) | bit 11 |  | extra |
| 2050.12 | O12 | Solar drain valve (bit 12) | bit 12 |  | extra |
| 2050.13 | O13 | Fan high speed (bit 13) | bit 13 |  | extra |
| 2050.14 | O14 | Fan low speed (bit 14) | bit 14 |  | extra |
| 2060 | O03 | EEV steps | raw |  | yes |
| 2061 | O04 | Compressor running cumulative time | raw = days |  | yes |
| 2066 | O06 | Superheat [K] | raw / 10 |  | yes |
| 2067 | O02 | Fan speed | raw / 10 |  | yes |

## Adding a register

1. Pick the register address and decoding type.
2. ESPHome: add a `template` sensor with `tesy::bwwp.temp(<reg>)` / `value(<reg>)` / `bit_set(<reg>, n)` / `nonzero(<reg>)` (registers 1000–1199 and 2000–2199 are stored).
3. Archived Node-RED flow: add an entry to the `SENSORS` list in the function node.
