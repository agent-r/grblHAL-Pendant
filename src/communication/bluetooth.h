#pragma once
#include <Arduino.h>
#include <BLEDevice.h>

extern BLEClient *pClient;


void Rx_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

void bluetoothInit();
bool bluetoothConnect();
void bluetoothDisconnect();
void bluetoothSend(const String& type, const String& cmd, const String& cmd_info);
// void bluetoothParse(String& JsonString);
void bluetoothParse(const uint8_t* data, size_t len);