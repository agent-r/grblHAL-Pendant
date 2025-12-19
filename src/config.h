#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <StreamUtils.h>            // TO READ CONFIG JSON FROM EEPROM
#include <ArduinoJson.h>            // JSON

#include "global.h"

extern JsonDocument configDoc;

void configLoad();
void configSave();