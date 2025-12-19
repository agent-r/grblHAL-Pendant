#pragma once
#include <Arduino.h>
#include <BLEDevice.h>


extern const int ConfigForms[2][5];
extern const int ConfigFields[12][5];

void config();
void configConnection();
void configConnectionSettings();
void configBluetoothAddress();


// String BLEDeviceList[11];
// uint8_t BLEAddressList[10][6];
// bool BLEDeviceFound;
// int BLEDeviceNum;


// uint16_t BLEScanTime;
extern BLEScan* pBLEScan;
 
void configBLE();

void configJogging();
void configJoggingX();
void configJoggingY();
void configJoggingZ();
void configJoggingA();
void configWorkspace();
void configProbe();
void configProbeOffset();
void configProbeDepth();
void configProbeSpeed();
void configProbeBackHeight();
void configProbeTime();
void configSleep();
void configBrightness();
void configBattery();
void configInfo();


