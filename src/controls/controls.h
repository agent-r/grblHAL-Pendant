#pragma once
#include <Arduino.h>
#include <TickTwo.h>
#include <JC_Button.h>

void controlsInit();

void checkEncoder();
bool EncoderChange();
int16_t readEncoder();
void resetEncoder();
void checkKeypad();
int readKeypad(bool buffered);
bool checkEnter();
bool checkEnterConfirm();
bool checkConfig();


void decreaseAxis();
void decreaseAxis();
void increaseAxis();
void setZero();
void gotoZero();
void probeZ();
void decreaseFactor();
void increaseFactor();
void homeAll();
void stop();
void unlock();
void enter();


float readBattery();
int percentageBattery(const float Voltage);

extern TickTwo EncoderTicker;
extern TickTwo KeypadTicker;

extern Button Button0;
extern Button Button1;
extern Button Button2;
extern Button Button3;
extern Button Button4;
extern Button Button5;
extern Button Button6;
extern Button Button7;
extern Button Button8;
extern Button Button9;
extern Button Button10;
extern Button Button11;
