#include <Arduino.h>
#include "communication/serial.h"
#include "global.h"

void serialInit() {
        // DEBUG SERIAL
        #if defined(SERIAL_DEBUG) || defined(SERIAL_DEBUG_IN) || defined(SERIAL_DEBUG_OUT) 
            Serial.begin(115200);
        #endif
}
