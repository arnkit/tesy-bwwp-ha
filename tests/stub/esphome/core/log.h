#pragma once
#include <cstdio>
#define ESP_LOGW(t,f,...) printf("W " f "\n",##__VA_ARGS__)
