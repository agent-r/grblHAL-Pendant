#pragma once
#include <Arduino.h>

void TFTUpdate();
void TFTSleep();
void TFTPrint(const byte Aim, String Content, const int Color);
void TFTMenuPrintAddress(const byte Aim, String Content1, String Content2, String Content3, const int ColorNormal, const int ColorHighlight);
void TFTPrepare();
void TFTClear();
void TFTMessage();

void TFTMenuPrepare();
void TFTMenuPrint(const byte Aim, String Content, const int Color);
