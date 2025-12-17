#include <Arduino.h>
#include <EEPROM.h>
#include <StreamUtils.h>            // TO READ CONFIG JSON FROM EEPROM
#include <ArduinoJson.h>            // JSON

#include "config.h"
#include "global.h"
#include "display/display.h"

JsonDocument configDoc;

void configInit() {

        // EEPROM
        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);

        DeserializationError jsonError = deserializeJson(configDoc, eepromConfigStream);
                if (jsonError) {
                        #ifdef SERIAL_DEBUG 
                                Serial.print(F("deserializeJson() failed: "));
                                Serial.println(jsonError.f_str());
                        #endif
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



        #ifdef SERIAL_DEBUG 

                Serial.println("[PENDANT] ------- EEPROM DATA ---------");

                Serial.println("[PENDANT] BLE HOST: " + String(BluetoothHost[0], HEX) + ":" + String(BluetoothHost[1], HEX) + ":" + String(BluetoothHost[2], HEX) + ":" + String(BluetoothHost[3], HEX) + ":" + String(BluetoothHost[4], HEX) + ":" + String(BluetoothHost[5], HEX));
                Serial.println("[PENDANT] BLE PIN: " + String(BluetoothPin));

                Serial.println("[PENDANT] X JOG SPEED: " + String(JogSpeed[0]));
                Serial.println("[PENDANT] Y JOG SPEED: " + String(JogSpeed[1]));
                Serial.println("[PENDANT] Z JOG SPEED: " + String(JogSpeed[2]));
                Serial.println("[PENDANT] A JOG SPEED: " + String(JogSpeed[3]));

                Serial.println("[PENDANT] PROBE OFFSET: " + String(ProbeOffset));
                Serial.println("[PENDANT] PROBE DEPTH: " + String(ProbeDepth));
                Serial.println("[PENDANT] PROBE SPEED: " + String(ProbeSpeed));
                Serial.println("[PENDANT] PROBE RISE: " + String(ProbeBackHeight));
                Serial.println("[PENDANT] PROBE TIME: " + String(ProbeTime));

                Serial.println("[PENDANT] SLEEP TIME: " + String(SleepTime) + "min");
                Serial.println("[PENDANT] BRIGHTNESS: " + String(TFT_BRIGHTNESS));

                Serial.println("[PENDANT] -------- EEPROM END ---------");

        #endif

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

        #ifdef SERIAL_DEBUG 
                Serial.print("[PENDANT] EEPROM JSON:    "); serializeJson(configDoc, Serial); Serial.println("");
        #endif
}
