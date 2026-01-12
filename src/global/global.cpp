#include <Arduino.h>
#include "global.h"
#include <vector>

CONFIG config = {
    .BluetoothHost = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .BluetoothPin = 1234,

    .ProbeOffset = 10,
    .ProbeDepth = 10,
    .ProbeSpeed = 100,
    .ProbeReturn = 10,
    .ProbeTime = 10,
    .ProbeAlarm = false,

    .SleepTime = 1,
    .TFTBrightness = 127,
    .workspace = 54
};

CNC cnc = {
    .axis = 
    {
        { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='X' },
        { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='Y' },
        { .position=0, .positionOld=0, .jogSpeed=100, .direction=-1, .name='Z' },
        { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='A' }
    },
    .activeAxisIndex = 0,
    .jogFactor = 1.0,
    .machineState = "starting\0         ",
    .machineStateOld = "starting\0         "
};

volatile SystemState systemState = STATE_STARTUP;

/*
void defaultsLoad() {

    // cnc.
    cnc.activeAxisIndex = 0;

    cnc.axis = {
    { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='X' },
    { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='Y' },
    { .position=0, .positionOld=0, .jogSpeed=100, .direction=-1, .name='Z' },
    { .position=0, .positionOld=0, .jogSpeed=100, .direction=1, .name='A' }
    };

    cnc.jogFactor = 1.0;
    snprintf(cnc.machineState, DISP_MAX_TEXT_MESSAGE_STATE, "%s", "starting");
    snprintf(cnc.machineStateOld, DISP_MAX_TEXT_MESSAGE_STATE, "%s", "starting");

    // config.
    uint8_t mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memcpy(config.BluetoothHost, mac, 6);
    config.BluetoothPin = 1234;

    config.ProbeOffset = 10;
    config.ProbeDepth = 10;
    config.ProbeSpeed = 100;
    config.ProbeReturn = 10;
    config.ProbeTime = 10;
    config.ProbeAlarm = false;

    config.SleepTime = 1;
    config.TFTBrightness = 127;

    config.workspace = 54;

}
    */
