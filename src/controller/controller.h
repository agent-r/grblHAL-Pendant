#pragma once
#include <Arduino.h>
#include "global/global.h"

void ControlTask(void *pv);
void sendGCode(const char* type, const char* gcode, const char* message);
void showMessage(const char* message);
void resetSleepTimer();