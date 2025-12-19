#pragma once
#include <Arduino.h>
#include "global.h"
#include <TickTwo.h>
#include <TFT_eSPI.h>


void TFTInit();

void TFTUpdate();
void TFTSleep();
void TFTPrint(const byte Aim, String Content, const int Color);
void TFTSetFontSize(const int size);
void TFTPrepare();
void TFTClear();
void TFTMessage();
void TFTBlink();

extern TickTwo TftTicker;
extern TickTwo BlinkTicker;
extern TickTwo MessageTicker;
extern TickTwo SleepTicker;

extern TFT_eSPI tft;