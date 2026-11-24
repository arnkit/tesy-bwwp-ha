// Host-side test of esphome/tesy_decoder.h using captured frames (no ESP needed).
// Build & run:  g++ -std=c++17 -Istub -I../esphome test_decoder.cpp -o test_decoder && ./test_decoder
#include "tesy_decoder.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#define bit(b) (1UL << (b))   // mimic Arduino.h's bit() macro: the decoder must not break because of it

static std::vector<uint8_t> load(const char *path) {
  std::ifstream in(path); std::string tok; std::vector<uint8_t> v;
  while (in >> tok) v.push_back((uint8_t) std::stoul(tok, nullptr, 16));
  return v;
}
static uint16_t crc16(const uint8_t *d, size_t n) {
  uint16_t c = 0xFFFF;
  for (size_t i = 0; i < n; i++) { c ^= d[i]; for (int k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ 0xA001 : (c >> 1); }
  return c;
}
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)

int main() {
  auto a = load("frames/frame_1001.hex"), b = load("frames/frame_1091.hex"), c = load("frames/frame_2001.hex");
  CHECK(a.size() == 189 && b.size() == 189 && c.size() == 189);

  // 1) garbage + three frames, fed byte by byte
  {
    tesy::Decoder d;
    for (uint8_t x : {0x00, 0xFF, 0x63, 0x10, 0x12}) d.feed(x);           // noise before the first frame
    for (auto *f : {&a, &b, &c}) for (uint8_t x : *f) d.feed(x);
    CHECK(d.frames_ok == 3);
    CHECK(d.crc_errors == 0);
    CHECK(d.modus() == "ECO");
    CHECK(d.clock() == "24.11.2026 09:45");
    CHECK(d.temp(1104) == 58.0f);
    CHECK(d.temp(2019) == 22.0f);   // AT
    CHECK(d.temp(2020) == 55.0f);   // BTT
    CHECK(d.temp(2021) == 53.0f);   // TTT
    CHECK(d.value(2060) == 240.0f); // EEV
    CHECK(d.value(2061) == 59.0f);  // compressor runtime [d]
    CHECK(d.raw(2050) == 0x0024);
    CHECK(d.bit_set(2050, 8).has_value() && !*d.bit_set(2050, 8));
    CHECK(!d.stale());
    g_ms += 200000;                  // 200 s without a frame
    CHECK(d.stale());
    CHECK(std::isnan(d.temp(2019)));
    g_ms = 1000;
  }
  // 2) a corrupted frame is rejected (CRC) and does not touch the registers
  {
    tesy::Decoder d;
    auto bad = c; bad[20] ^= 0x55;
    for (uint8_t x : bad) d.feed(x);
    CHECK(d.frames_ok == 0 && d.crc_errors >= 1);
    CHECK(!d.ok(2019));
  }
  // 3) address based: the same data announced one register higher must appear one register higher
  {
    tesy::Decoder d;
    auto s = c; s[3] = (uint8_t)(s[3] + 1);                                 // start address 2001 -> 2002
    uint16_t crc = crc16(s.data(), s.size() - 2); s[s.size() - 2] = crc & 0xFF; s[s.size() - 1] = crc >> 8;
    for (uint8_t x : s) d.feed(x);
    CHECK(d.frames_ok == 1);
    CHECK(d.raw(2020) == (uint16_t)((c[7 + 2 * 18] << 8) | c[8 + 2 * 18]));   // was reg 2019, now 2020
  }
  printf("all tests passed\n");
  return 0;
}
