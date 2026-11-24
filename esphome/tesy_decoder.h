#pragma once
// =============================================================================
// TESY Aquathermica Pro (BWWP) - passive RS485 sniffer for ESPHome  (v3)
//
// The bus carries plain Modbus-RTU (function code 0x10 "Write Multiple Registers"),
// 9600 baud, 8 data bits, no parity, **1 stop bit (8N1)**:
//
//   [Unit 0x63] [0x10] [start addr hi lo] [quantity hi lo] [byte count] [registers, 16 bit big endian ...] [CRC lo hi]
//
// The unit sends three blocks of 90 registers each in turn (one block every ~2.7 s):
//   start address 1001, 1091 and 2001  (189 bytes per frame incl. CRC)
//
// A Waveshare gateway turns this into Modbus-TCP (it strips the CRC) - that is what the
// Node-RED flow in this repository reads. This decoder works like that flow, but directly
// on the RTU byte stream:
//   * A register is stored by its ADDRESS from the frame header, not by its byte position.
//     If the unit moves registers around or changes the block size, decoding keeps working.
//   * Every frame is verified with CRC16; noise/garbage is discarded (byte-wise resync).
//
// IMPORTANT: with 2 stop bits configured on the ESP, bytes are lost (mostly 0x00) because
// the bus only sends 1 stop bit. The stream then looks "compressed" and register positions
// seem to drift. The YAML therefore MUST use stop_bits: 1.
// =============================================================================
#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include "esphome/core/hal.h"        // millis()
#include "esphome/core/optional.h"
#include "esphome/core/log.h"

namespace tesy {

static constexpr uint32_t STALE_MS = 180000;   // like EXPIRE_S in the Node-RED flow: 180 s without a frame -> "no data"

class Decoder {
 public:
  // --- counters for diagnostics ---
  uint32_t frames_ok  = 0;   // frames with a valid CRC
  uint32_t crc_errors = 0;   // frames with a plausible structure but a wrong CRC (= transmission errors)
  uint32_t bytes_rx   = 0;   // all received bytes
  uint32_t last_ms    = 0;   // millis() of the last valid frame (0 = none yet)

  // Feed every received byte in here (from the uart lambda).
  void feed(uint8_t b) {
    bytes_rx++;
    buf_.push_back(b);
    if (buf_.size() > 512) buf_.clear();       // safety valve
    parse_();
  }

  // ---- access to register values ----
  bool stale() const { return last_ms == 0 || (millis() - last_ms) > STALE_MS; }

  // Is there a fresh value for register r?
  bool ok(uint16_t r) const {
    int i = slot_(r);
    if (i < 0 || !have_(i)) return false;
    uint32_t t = t_[i / SLOT_REGS];
    return t != 0 && (millis() - t) <= STALE_MS;
  }
  bool has(uint16_t r) const { return ok(r); }
  uint16_t raw(uint16_t r) const {
    int i = slot_(r);
    return i < 0 ? 0 : regs_[i];
  }

  // Temperature: T[degC] = raw/2 - 30  (same as in the flow)
  float temp(uint16_t r) const { return ok(r) ? raw(r) / 2.0f - 30.0f : NAN; }
  // Raw value, optionally multiplied/divided (like mul/div in the flow)
  float value(uint16_t r, float mul = 1.0f, float div = 1.0f) const {
    return ok(r) ? raw(r) * mul / div : NAN;
  }
  // Bit n of a register (reg 2050: O08..O14). Do not name it "bit": Arduino.h defines a macro bit(b)!
  esphome::optional<bool> bit_set(uint16_t r, uint8_t n) const {
    if (!ok(r)) return {};
    return ((raw(r) >> n) & 1) != 0;
  }
  // Register != 0
  esphome::optional<bool> nonzero(uint16_t r) const {
    if (!ok(r)) return {};
    return raw(r) != 0;
  }

  // Operating mode: reg 1012, "Off" if reg 1011 == 0
  std::string modus() const {
    if (!ok(1012) || !ok(1011)) return "";
    if (raw(1011) == 0) return "Off";
    switch (raw(1012)) {
      case 0: return "Smart";
      case 2: return "ECO";
      case 3: return "Auto";
      case 4: return "High demand";
      case 7: return "Vacation";
      case 8: return "Boost";
      default: return "Unknown";
    }
  }

  // Device clock: reg 1152 minute, 1153 hour, 1154 day, 1155 month, 1156 year (2 digits)
  std::string clock() const {
    for (uint16_t r = 1152; r <= 1156; r++) if (!ok(r)) return "";
    char s[48];
    snprintf(s, sizeof(s), "%02u.%02u.20%02u %02u:%02u", (unsigned) raw(1154), (unsigned) raw(1155),
             (unsigned) raw(1156), (unsigned) raw(1153), (unsigned) raw(1152));
    return s;
  }

 private:
  // Register storage: two windows 1000..1199 and 2000..2199 (200 registers each = 800 bytes of RAM)
  static constexpr uint16_t WIN_REGS = 200;
  static constexpr uint16_t SLOT_REGS = 50;                       // freshness timestamp per 50 registers
  static constexpr uint16_t N_SLOTS = 2 * WIN_REGS / SLOT_REGS;   // = 8

  std::vector<uint8_t> buf_;
  uint16_t regs_[2 * WIN_REGS] = {0};
  uint8_t  have_bits_[2 * WIN_REGS / 8] = {0};
  uint32_t t_[N_SLOTS] = {0};

  static int slot_(uint16_t r) {
    if (r >= 1000 && r < 1000 + WIN_REGS) return r - 1000;
    if (r >= 2000 && r < 2000 + WIN_REGS) return WIN_REGS + (r - 2000);
    return -1;
  }
  bool have_(int i) const { return (have_bits_[i >> 3] >> (i & 7)) & 1; }
  void set_have_(int i) { have_bits_[i >> 3] |= (1 << (i & 7)); }

  static uint16_t crc16_(const uint8_t *d, size_t n) {
    uint16_t c = 0xFFFF;
    for (size_t i = 0; i < n; i++) {
      c ^= d[i];
      for (int k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ 0xA001 : (c >> 1);
    }
    return c;
  }

  // Looks for Modbus-RTU FC10 frames in the byte stream; on errors one byte is dropped (resync, like in the flow).
  void parse_() {
    while (buf_.size() >= 9) {
      const uint8_t *f = buf_.data();
      uint16_t qty = (f[4] << 8) | f[5];               // number of registers
      uint8_t bc = f[6];                               // byte count
      if (f[1] != 0x10 || qty == 0 || qty > 123 || bc != qty * 2) { buf_.erase(buf_.begin()); continue; }
      size_t total = 7 + bc + 2;                       // header + data + CRC
      if (buf_.size() < total) return;                 // frame not complete yet
      uint16_t crc = f[total - 2] | (f[total - 1] << 8);
      if (crc != crc16_(f, total - 2)) {
        crc_errors++;
        if (crc_errors <= 5 || crc_errors % 100 == 0)
          ESP_LOGW("tesy", "CRC error (%u): frame dropped. Stop bits = 1? Baud rate/parity?", (unsigned) crc_errors);
        buf_.erase(buf_.begin());
        continue;
      }
      uint16_t addr = (f[2] << 8) | f[3];
      uint32_t now = millis();
      if (now == 0) now = 1;
      for (uint16_t i = 0; i < qty; i++) {
        int s = slot_(addr + i);
        if (s < 0) continue;                           // ignore registers outside the windows
        regs_[s] = (f[7 + 2 * i] << 8) | f[8 + 2 * i];
        set_have_(s);
        t_[s / SLOT_REGS] = now;
      }
      frames_ok++;
      last_ms = now;
      buf_.erase(buf_.begin(), buf_.begin() + total);
    }
  }
};

inline Decoder bwwp;   // global instance, addressed as tesy::bwwp in the YAML

}  // namespace tesy
