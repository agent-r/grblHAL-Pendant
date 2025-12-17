#pragma once
#include <Arduino.h>
#include <BLEDevice.h>

extern BLEClient *pClient;

void bluetoothInit();
bool bluetoothConnect();
void bluetoothDisconnect();
void bluetoothSend(const String& type, const String& cmd, const String& cmd_info);
void bluetoothParse(String& JsonString);