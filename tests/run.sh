#!/bin/sh
# Build and run the host test (needs only a C++17 compiler).
set -e
cd "$(dirname "$0")"
g++ -std=c++17 -Wall -Istub -I../esphome test_decoder.cpp -o /tmp/test_decoder
/tmp/test_decoder
