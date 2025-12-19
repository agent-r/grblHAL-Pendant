#pragma once
#include <Arduino.h>
#include <stdarg.h>
#include <stddef.h>
#include "global.h"

void debug(const char* msg);
void debugf(uint8_t flags, const char* fmt, ...);


