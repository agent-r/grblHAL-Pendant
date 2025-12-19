#include <Arduino.h>
#include "communication/serial.h"
#include "global.h"

void serialInit() {
        #ifdef DEBUG_SERIAL 
            Serial.begin(115200);
        #endif
}
