#include <Arduino.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <EEPROM.h>

#include "global/global.h"
#include "debug/debug.h"


void eepromLoad()
{
        JsonDocument eepromDoc;

        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);

        DeserializationError jsonError = deserializeJson(eepromDoc, eepromConfigStream);

        /*
        if (jsonError)
        {
                debugLog(DBG_ERROR, "EEPROM", "DESERIALIZE FAILED - SET TO DEFAULTS");

                uint8_t mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                memcpy(config.BluetoothHost, mac, 6);
                config.BluetoothPin = 0000;

                cnc.axis[0].jogSpeed = 100;
                cnc.axis[1].jogSpeed = 100;
                cnc.axis[2].jogSpeed = 100;
                cnc.axis[3].jogSpeed = 100;

                config.ProbeOffset = 10;
                config.ProbeDepth = 10;
                config.ProbeSpeed = 100;
                config.ProbeReturn = 10;
                config.ProbeTime = 10;

                config.SleepTime = 1;
                config.TFTBrightness = 127;
        }
        else
        */

        if (!jsonError)
        {
                copyArray(eepromDoc[EEBluetoothHost], config.BluetoothHost);
                config.BluetoothPin = eepromDoc[EEBluetoothPin];

                cnc.axis[0].jogSpeed = eepromDoc[EEJogSpeed][0];
                cnc.axis[1].jogSpeed = eepromDoc[EEJogSpeed][1];
                cnc.axis[2].jogSpeed = eepromDoc[EEJogSpeed][2];
                cnc.axis[3].jogSpeed = eepromDoc[EEJogSpeed][3];

                config.ProbeOffset = eepromDoc[EEProbeOffset];
                config.ProbeDepth = eepromDoc[EEProbeDepth];
                config.ProbeSpeed = eepromDoc[EEProbeSpeed];
                config.ProbeReturn = eepromDoc[EEProbeBackHeight];
                config.ProbeTime = eepromDoc[EEProbeTime];

                config.SleepTime = eepromDoc[EESleepTime];
                config.TFTBrightness = eepromDoc[EEBrightness];
        }

        debugLog(DBG_INFO, "EEPROM", "------- EEPROM DATA ---------");
        DEBUGF(DBG_INFO, "EEPROM", "BLE HOST: %02X:%02X:%02X:%02X:%02X:%02X", config.BluetoothHost[0], config.BluetoothHost[1], config.BluetoothHost[2], config.BluetoothHost[3], config.BluetoothHost[4], config.BluetoothHost[5]);
        DEBUGF(DBG_INFO, "EEPROM", "BLE PIN: %d", config.BluetoothPin);

        DEBUGF(DBG_INFO, "EEPROM", "X JOG SPEED: %d", cnc.axis[0].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "Y JOG SPEED: %d", cnc.axis[1].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "Z JOG SPEED: %d", cnc.axis[2].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "A JOG SPEED: %d", cnc.axis[3].jogSpeed);

        DEBUGF(DBG_INFO, "EEPROM", "PROBE OFFSET: %.2f", config.ProbeOffset);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE DEPTH: %.2f", (float)config.ProbeDepth);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE SPEED: %.2f", (float)config.ProbeSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE RISE: %.2f", (float)config.ProbeReturn);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE TIME: %d", config.ProbeTime);

        DEBUGF(DBG_INFO, "EEPROM", "SLEEP TIME: %d", config.SleepTime);
        DEBUGF(DBG_INFO, "EEPROM", "BRIGHTNESS: %d", config.TFTBrightness);

        debugLog(DBG_INFO, "EEPROM", "-------- EEPROM END ---------");
}

void eepromSave()
{

        JsonDocument eepromDoc;

        eepromDoc[EEBluetoothHost].clear();
        copyArray(config.BluetoothHost, eepromDoc[EEBluetoothHost]);
        eepromDoc[EEBluetoothPin] = config.BluetoothPin;
        eepromDoc[EEJogSpeed][0] = cnc.axis[0].jogSpeed;
        eepromDoc[EEJogSpeed][1] = cnc.axis[1].jogSpeed;
        eepromDoc[EEJogSpeed][2] = cnc.axis[2].jogSpeed;
        eepromDoc[EEJogSpeed][3] = cnc.axis[3].jogSpeed;
        eepromDoc[EEProbeOffset] = config.ProbeOffset;
        eepromDoc[EEProbeDepth] = config.ProbeDepth;
        eepromDoc[EEProbeSpeed] = config.ProbeSpeed;
        eepromDoc[EEProbeBackHeight] = config.ProbeReturn;
        eepromDoc[EEProbeTime] = config.ProbeTime;
        eepromDoc[EESleepTime] = config.SleepTime;
        eepromDoc[EEBrightness] = config.TFTBrightness;

        EEPROM.begin(EEPROM_LENGTH);
        EepromStream eepromConfigStream(0, EEPROM_LENGTH);
        serializeJson(eepromDoc, eepromConfigStream);
        EEPROM.commit();

        // char jsonBuf[EEPROM_LENGTH];                                         // CRASHES !!!
        // serializeJson(eepromDoc, jsonBuf, sizeof(jsonBuf));                  // CRASHES !!!
        // DEBUGF(DBG_INFO, "EEPROM", "%s", jsonBuf);                           // CRASHES !!!

        debugLog(DBG_INFO, "EEPROM", "------- EEPROM DATA SAVED ---------");
        DEBUGF(DBG_INFO, "EEPROM", "BLE HOST: %02X:%02X:%02X:%02X:%02X:%02X", config.BluetoothHost[0], config.BluetoothHost[1], config.BluetoothHost[2], config.BluetoothHost[3], config.BluetoothHost[4], config.BluetoothHost[5]);

        DEBUGF(DBG_INFO, "EEPROM", "BLE PIN: %d", config.BluetoothPin);

        DEBUGF(DBG_INFO, "EEPROM", "X JOG SPEED: %d", cnc.axis[0].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "Y JOG SPEED: %d", cnc.axis[1].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "Z JOG SPEED: %d", cnc.axis[2].jogSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "A JOG SPEED: %d", cnc.axis[3].jogSpeed);

        DEBUGF(DBG_INFO, "EEPROM", "PROBE OFFSET: %f", config.ProbeOffset);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE DEPTH: %d mm", config.ProbeDepth);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE SPEED: %d mm/sec", config.ProbeSpeed);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE RISE: %d mm", config.ProbeReturn);
        DEBUGF(DBG_INFO, "EEPROM", "PROBE TIME: %d sec", config.ProbeTime);

        DEBUGF(DBG_INFO, "EEPROM", "SLEEP TIME: %d min", config.SleepTime);
        DEBUGF(DBG_INFO, "EEPROM", "BRIGHTNESS: %d", config.TFTBrightness);

        debugLog(DBG_INFO, "EEPROM", "-------- EEPROM END ---------");
}
