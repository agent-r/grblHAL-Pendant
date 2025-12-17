#pragma once
#include <Arduino.h>


void TFTConfigPrepare();
void TFTConfigPrint(const byte Aim, String Content, const int Color);
byte TFTConfigMenu(const String* Content, const int Length);
float TFTConfigValue(const String Title, const int Min, const int Max, float Value, const int Factor, const char* Unit, const byte leadingZeros);
String TFTConfigString(const String Title, const String oldString);
String TFTConfigStringChar(int Position);
int TFTConfigStringCharPosX(int Position);
int TFTConfigStringCharPosY(int Position);
void TFTConfigInfo(const String* Content, const int Length);
