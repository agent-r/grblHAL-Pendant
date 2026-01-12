#pragma once
#include <Arduino.h>
#include "global/global.h"

void debugLog(DebugLevel lvl, const char *src, const char *msg);
void DebugTask(void *pv);
