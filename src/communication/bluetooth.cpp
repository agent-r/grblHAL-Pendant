#include <Arduino.h>
#include <BLEDevice.h>
#include <ArduinoJson.h>

#include "bluetooth.h"
#include "global.h"
#include "controls/controls.h"
#include "display/configmenu.h"
#include "display/display.h"
#include "communication/debug.h"


static BLEUUID SERVICE_UUID("825aeb6e-7e1d-4973-9c75-30c042c4770c");
static BLEUUID TX_CHARACTERISTIC_UUID("24259347-9d86-4c67-a9ae-84f6a7f0c90d");
static BLEUUID RX_CHARACTERISTIC_UUID("b52e05ac-8a8a-4880-85c7-bd3e6a32dc0e");
static BLERemoteCharacteristic* Tx_Characteristic;
static BLERemoteCharacteristic* Rx_Characteristic;

BLEClient *pClient = nullptr;


// Callback When the BLE Bridge sends a new state
static void Rx_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {

//        char* Rx = (char*)pData;
//        String Rx_str = String(Rx);

        String Rx_str((char*)pData, length);
        bluetoothParse(Rx_str);

}


class MyClientCallback : public BLEClientCallbacks {

        void onConnect(BLEClient* pclient) {
                BLEconnected = true;
        }

        void onDisconnect(BLEClient* pclient) {
                BLEconnected = false;
                #ifdef SERIAL_DEBUG
                        Serial.println("[PENDANT] BLE disconnected");
                #endif
        }

};



void bluetoothInit() {
        #ifdef SERIAL_DEBUG
                Serial.println("[PENDANT] Setting up BLE Connection");
        #endif
        BLEDevice::init("");
}


bool bluetoothConnect() {

        if (BLEconnected) {
                return true;
        }
        else {

                if (checkConfig()) { config(); return false; };

                TFTPrint(MessageField, "BLE Connecting..", TFT_COLOR_STA_ERR);

                pClient = BLEDevice::createClient();
        
                // Set Callbacks for BLE Client
                pClient->setClientCallbacks(new MyClientCallback());

                // Scan for GRBLHAL Server first ??

                // connect to GRBLHAL Server
                if (pClient->connect(BluetoothHost) == false) { 
                        if (checkConfig()) { config(); }        // chance to enter config
                        return(false); 
                }

                BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                if (pRemoteService == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find our service UUID: ");
                                Serial.println("[PENDANT] " + String(SERVICE_UUID.toString().c_str()));
                        #endif
                        return(false);
                }
        
                Rx_Characteristic = pRemoteService->getCharacteristic(RX_CHARACTERISTIC_UUID);
                if (Rx_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX characteristic UUID");
                        #endif
                        return false;
                }
                Rx_Characteristic->registerForNotify(Rx_NotifyCallback); //Assign callback functions for the Characteristics

                
                Tx_Characteristic = pRemoteService->getCharacteristic(TX_CHARACTERISTIC_UUID);
                if (Tx_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find TX characteristic UUID");
                        #endif
                        return false;
                }

                resetEncoder(); // needed?
                return true;
        }
}


void bluetoothDisconnect() {
        if (pClient != nullptr) {
                pClient->disconnect();
        }
}


void bluetoothParse(String& JsonString) {

        debug("[PENDANT] RECEIVED:    " + JsonString);

        // DynamicJsonDocument JsonIn(128); // deprecated
        JsonDocument JsonIn;
        DeserializationError error = deserializeJson(JsonIn, JsonString);

        if (error) {
                debug("[PENDANT] WARNING: DESERIALIZATION ERROR:   " + String(error.f_str()));
        }
        else {
                // if (JsonIn.containsKey("wx")) {  is<JsonVariant>()
                if (JsonIn["wx"].is<JsonVariant>()) {
                        wx = JsonIn["wx"];
                        if (wx != wxold) { wxchange = true; wxold = wx; }
                }
                if (JsonIn["wy"].is<JsonVariant>()) {
                        wy = JsonIn["wy"];
                        if (wy != wyold) { wychange = true; wyold = wy; }
                }
                if (JsonIn["wz"].is<JsonVariant>()) {
                        wz = JsonIn["wz"];
                        if (wz != wzold) { wzchange = true; wzold = wz; }
                }
                if (JsonIn["wa"].is<JsonVariant>()) {
                        wa = JsonIn["wa"];
                        if (wa != waold) { wachange = true; waold = wa; }
                }
                if (JsonIn["state"].is<JsonVariant>()) {
                        const char* statechar = JsonIn["state"];
                        state = String(statechar);
                        if (state != stateold) { statechange = true; stateold = state; }
                        if (state == "Alarm") { Probe_Alarm = true; }
                        if (state == "Run") { SleepTicker.start(); }
                }
                
                #ifdef SERIAL_DEBUG_IN
                        debug("[PENDANT] RECEIVED & PARSED:   WX: " + String(wx) + "   WY: " + String(wy) + "   WZ: " + String(wz) + "   WA: " + String(wa) + "   STATE: " + state);
                #endif
        }
}


void bluetoothSend(const String& type, const String& cmd, const String& cmd_info) {

        String cmd_json = "{\"" + type + "\":\"" + cmd + "\"}";

        if (bluetoothConnect())
        {
                Tx_Characteristic->writeValue(cmd_json.c_str(), cmd.length());

                #ifdef SERIAL_DEBUG_OUT
                        Serial.println(cmd_json);
                #endif

                TFTPrint(MessageField, cmd_info, TFT_COLOR_MSG_NRM);
        }
        else {
                TFTPrint(MessageField, "NO CONNECTION. HOLD!", TFT_COLOR_MSG_ERR);
        }
}


