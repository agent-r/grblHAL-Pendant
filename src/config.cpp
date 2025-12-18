#include <Arduino.h>
#include <EEPROM.h>
#include <StreamUtils.h>            // TO READ CONFIG JSON FROM EEPROM
#include <ArduinoJson.h>            // JSON

#include "config.h"
#include "global.h"
#include "display/display.h"
#include "communication/debug.h"

JsonDocument configDoc;

void configInit() {

        // EEPROM
        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);

        DeserializationError jsonError = deserializeJson(configDoc, eepromConfigStream);
                if (jsonError) {

                        debug("deserializeJson() failed:");

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

                debug("[PENDANT] ------- EEPROM DATA ---------");

                debug("[PENDANT] BLE HOST: " + String(BluetoothHost[0], HEX) + ":" + String(BluetoothHost[1], HEX) + ":" + String(BluetoothHost[2], HEX) + ":" + String(BluetoothHost[3], HEX) + ":" + String(BluetoothHost[4], HEX) + ":" + String(BluetoothHost[5], HEX));
                debug("[PENDANT] BLE PIN: " + String(BluetoothPin));

                debug("[PENDANT] X JOG SPEED: " + String(JogSpeed[0]));
                debug("[PENDANT] Y JOG SPEED: " + String(JogSpeed[1]));
                debug("[PENDANT] Z JOG SPEED: " + String(JogSpeed[2]));
                debug("[PENDANT] A JOG SPEED: " + String(JogSpeed[3]));

                debug("[PENDANT] PROBE OFFSET: " + String(ProbeOffset));
                debug("[PENDANT] PROBE DEPTH: " + String(ProbeDepth));
                debug("[PENDANT] PROBE SPEED: " + String(ProbeSpeed));
                debug("[PENDANT] PROBE RISE: " + String(ProbeBackHeight));
                debug("[PENDANT] PROBE TIME: " + String(ProbeTime));

                debug("[PENDANT] SLEEP TIME: " + String(SleepTime) + "min");
                debug("[PENDANT] BRIGHTNESS: " + String(TFT_BRIGHTNESS));

                debug("[PENDANT] -------- EEPROM END ---------");

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

        debug("[PENDANT] EEPROM JSON SAVED");

        if (SleepTime > 0) {
                SleepTicker.interval(SleepTime * 60000);
                SleepTicker.start();
        }
        else {
                SleepTicker.interval(10000);
                SleepTicker.stop();
        }

}
