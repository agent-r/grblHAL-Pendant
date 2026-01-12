#pragma once
#include <Arduino.h>
#include <BLEDevice.h>

extern const int ConfigForms[2][5];
extern const int ConfigFields[12][5];

enum ValueType
{
    VALUE_INT8,
    VALUE_UINT8,
    VALUE_INT16,
    VALUE_UINT16,
    VALUE_INT32,
    VALUE_FLOAT
};

struct MenuValueDesc
{
    const char *title;
    ValueType type;
    void *valuePtr;
    float minValue;
    float maxValue;
    float step;
    uint8_t decimals;
    const char *predescriptor;
    const char *unit;
    uint8_t leadingZeros;
    void (*onChange)(int newValue);
    void (*onExit)();
};

void updateMenu(InputEvent &inEv);

void initMenu();

void actionBack();
void actionExit();

void actionBLEScan();
void actionBLEAddress();
void actionBLEPin();

void actionJoggingX();
void actionJoggingY();
void actionJoggingZ();
void actionJoggingA();

void actionWorkspace();
void setWorkspace();

void actionProbeOffset();
void actionProbeDepth();
void actionProbeSpeed();
void actionProbeReturn();
void actionProbeTime();

void actionSleepTime();

void actionBrightness();
void setBrightness(int brightness);

void actionInfoDisplay();

float readValue(const MenuValueDesc *d);
void writeValue(const MenuValueDesc *d, float v);

void menuSendDisplay();
void menuPrepareDisplay();

void menuSendValueDisplay();
void menuSendBLEAddrDisplay();

void handleBLEScanEvent(const BLEScanEvent &ev);
void sendBLEScanMenuToDisplay();
void actionBLEScan();
