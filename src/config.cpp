#include <Arduino.h>
#include <EEPROM.h>
#include <StreamUtils.h>            // TO READ CONFIG JSON FROM EEPROM
#include <ArduinoJson.h>            // JSON

#include "config.h"
#include "global.h"
#include "display/display.h"
#include "communication/debug.h"

JsonDocument configDoc;

void configLoad() {

        // EEPROM
        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);

        DeserializationError jsonError = deserializeJson(configDoc, eepromConfigStream);
                if (jsonError) {

                        debug("EEPROM DESERIALIZE FAILED");

                        // set defaults:
                        memset(BluetoothHost, 9, sizeof(BluetoothHost));
                        BluetoothPin = 9999;
                        memset(JogSpeed, 100, sizeof(JogSpeed));
                        ProbeOffset = 10;
                        ProbeDepth = 10;
                        ProbeSpeed = 100;
                        ProbeBackHeight = 10;
                        ProbeTime = 10;
                        SleepTime = 1;
                        SleepTicker.interval(SleepTime * 60000);
                        TFT_BRIGHTNESS = 127;
                }
                else {
                        copyArray(configDoc[EEBluetoothHost], BluetoothHost);
                        BluetoothPin = configDoc[EEBluetoothPin];
                        copyArray(configDoc[EEJogSpeed], JogSpeed);
                        ProbeOffset = configDoc[EEProbeOffset];

                        ProbeDepth = configDoc[EEProbeDepth];
                        ProbeSpeed = configDoc[EEProbeSpeed];
                        ProbeBackHeight = configDoc[EEProbeBackHeight];
                        ProbeTime = configDoc[EEProbeTime];

                        SleepTime = configDoc[EESleepTime];
                        SleepTicker.interval(SleepTime * 60000);

                        TFT_BRIGHTNESS = configDoc[EEBrightness];
                }

                debug("------- EEPROM DATA ---------");
                debugf(DEBUG_FLAG_BOTH, "BLE HOST: %02X:%02X:%02X:%02X:%02X:%02X", BluetoothHost[0], BluetoothHost[1], BluetoothHost[2], BluetoothHost[3], BluetoothHost[4], BluetoothHost[5]);
                  
                debugf(DEBUG_FLAG_BOTH, "BLE PIN: %d", BluetoothPin);
                  
                debugf(DEBUG_FLAG_BOTH, "X JOG SPEED: %d", JogSpeed[0]);
                debugf(DEBUG_FLAG_BOTH, "Y JOG SPEED: %d", JogSpeed[1]);
                debugf(DEBUG_FLAG_BOTH, "Z JOG SPEED: %d", JogSpeed[2]);
                debugf(DEBUG_FLAG_BOTH, "A JOG SPEED: %d", JogSpeed[3]);
        
                debugf(DEBUG_FLAG_BOTH, "PROBE OFFSET: %f", ProbeOffset);
                debugf(DEBUG_FLAG_BOTH, "PROBE DEPTH: %f", ProbeDepth);
                debugf(DEBUG_FLAG_BOTH, "PROBE SPEED: %f", ProbeSpeed);
                debugf(DEBUG_FLAG_BOTH, "PROBE RISE: %f", ProbeBackHeight);
                debugf(DEBUG_FLAG_BOTH, "PROBE TIME: %d", ProbeTime);
                  
                debugf(DEBUG_FLAG_BOTH, "SLEEP TIME: %d DEBUG_FLAG_BOTH, min", SleepTime);
                debugf(DEBUG_FLAG_BOTH, "BRIGHTNESS: %d DEBUG_FLAG_BOTH,", TFT_BRIGHTNESS);

                debug("-------- EEPROM END ---------");

        }


void configSave() {

        configDoc[EEBluetoothHost].clear();
        copyArray(BluetoothHost, configDoc[EEBluetoothHost]);
        configDoc[EEBluetoothPin] = BluetoothPin;
        configDoc[EEJogSpeed].clear();   // löscht alle alten Werte
        copyArray(JogSpeed, configDoc[EEJogSpeed]);
        configDoc[EEProbeOffset] = ProbeOffset;
        configDoc[EEProbeDepth] = ProbeDepth;
        configDoc[EEProbeSpeed] = ProbeSpeed;
        configDoc[EEProbeBackHeight] = ProbeBackHeight;
        configDoc[EEProbeTime] = ProbeTime;
        configDoc[EESleepTime] = SleepTime;
        configDoc[EEBrightness] = TFT_BRIGHTNESS;

        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);
        serializeJson(configDoc, eepromConfigStream);
        EEPROM.commit();

        debug("------- EEPROM DATA SAVED ---------");
        debugf(DEBUG_FLAG_BOTH, "BLE HOST: %02X:%02X:%02X:%02X:%02X:%02X", BluetoothHost[0], BluetoothHost[1], BluetoothHost[2], BluetoothHost[3], BluetoothHost[4], BluetoothHost[5]);
          
        debugf(DEBUG_FLAG_BOTH, "BLE PIN: %d", BluetoothPin);
          
        debugf(DEBUG_FLAG_BOTH, "X JOG SPEED: %d", JogSpeed[0]);
        debugf(DEBUG_FLAG_BOTH, "Y JOG SPEED: %d", JogSpeed[1]);
        debugf(DEBUG_FLAG_BOTH, "Z JOG SPEED: %d", JogSpeed[2]);
        debugf(DEBUG_FLAG_BOTH, "A JOG SPEED: %d", JogSpeed[3]);
          
        debugf(DEBUG_FLAG_BOTH, "PROBE OFFSET: %f", ProbeOffset);
        debugf(DEBUG_FLAG_BOTH, "PROBE DEPTH: %f", ProbeDepth);
        debugf(DEBUG_FLAG_BOTH, "PROBE SPEED: %f", ProbeSpeed);
        debugf(DEBUG_FLAG_BOTH, "PROBE RISE: %f", ProbeBackHeight);
        debugf(DEBUG_FLAG_BOTH, "PROBE TIME: %d", ProbeTime);
          
        debugf(DEBUG_FLAG_BOTH, "SLEEP TIME: %d min", SleepTime);
        debugf(DEBUG_FLAG_BOTH, "BRIGHTNESS: %d", TFT_BRIGHTNESS);

        debug("-------- EEPROM END ---------");

        if (SleepTime > 0) {
                SleepTicker.interval(SleepTime * 60000);
                SleepTicker.start();
        }
        else {
                SleepTicker.interval(10000);
                SleepTicker.stop();
        }

}
