
#include <Arduino.h>
#include "communication/debug.h"
#include "global.h"
// #include "communication/bluetooth.h"

void debug(const String& msg) {
        #ifdef SERIAL_DEBUG
                Serial.println(msg);
        #endif
        /*
        #ifdef MESSAGE_DEBUG
                if (BLEconnected) {
                        bluetoothSend("msg", msg, ""); 
                }
        #endif
        */
}

