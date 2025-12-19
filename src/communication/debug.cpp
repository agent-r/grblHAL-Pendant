
#include <Arduino.h>
#include <stdarg.h>
#include "communication/debug.h"
#include "global.h"

#if DEBUG_BLE
        #include "communication/bluetooth.h"
#endif

void debug(const char* msg) {
        #if DEBUG_SERIAL
                Serial.printf("[PENDANT] %s\n", msg);
        #endif
        #if DEBUG_BLE
                // bluetoothSend("msg", msg, "");
        #endif
}

void debugf(uint8_t flags, const char* fmt, ...) {

        
        #if !DEBUG_SERIAL && !DEBUG_BLE
                return;
        #endif

        static char buf[256];
        static const char prefix[] = "[PENDANT] ";

        va_list args;
        va_start(args, fmt);

        size_t prefixLen = snprintf(buf, sizeof(buf), "%s", prefix);
        vsnprintf(buf + prefixLen, sizeof(buf) - prefixLen, fmt, args);
        va_end(args);

        // Senden nur, wenn gewollt
        #if DEBUG_SERIAL
                if (flags & DEBUG_FLAG_SERIAL) Serial.println(buf);
        #endif
        #if DEBUG_BLE
                if (flags & DEBUG_FLAG_BLE) bluetoothSend("msg", buf, "");
        #endif
        
}