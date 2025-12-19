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


struct BlePacket {
        uint8_t data[BLE_MAX_PACKET];
        size_t len;
    };


// Callback When the BLE Bridge sends a new state
void Rx_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {

        BlePacket pkt;
        pkt.len = length > BLE_MAX_PACKET ? BLE_MAX_PACKET : length;
        memcpy(pkt.data, pData, pkt.len);
        xQueueSendFromISR(bleQueue, &pkt, NULL);

}


class MyClientCallback : public BLEClientCallbacks {

        void onConnect(BLEClient* pclient) {
                BLEconnected = true;
                debug("BLE CONNECTED");
        }

        void onDisconnect(BLEClient* pclient) {
                BLEconnected = false;
                debug("BLE DISCONNECTED");
        }

};



void bluetoothInit() {
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

                // connect to GRBLHAL Server
                if (pClient->connect(BluetoothHost) == false) { 
                        if (checkConfig()) { config(); }        // chance to enter config
                        return(false); 
                }

                pClient->setMTU(247); // BLE-Buffer vergrößeren.


                BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                if (pRemoteService == nullptr) {
                        debugf(DEBUG_FLAG_SERIAL, "BLUETOOTH FAILED TO FIND SERVICE UUID: %s", SERVICE_UUID.toString());
                        return(false);
                }
        
                Rx_Characteristic = pRemoteService->getCharacteristic(RX_CHARACTERISTIC_UUID);
                if (Rx_Characteristic == nullptr) {
                        debugf(DEBUG_FLAG_SERIAL, "BLUETOOTH FAILED TO FIND RX CHARACTERISTIC UUID");
                        return false;
                }
                Rx_Characteristic->registerForNotify(Rx_NotifyCallback); //Assign callback functions for the Characteristics

                
                Tx_Characteristic = pRemoteService->getCharacteristic(TX_CHARACTERISTIC_UUID);
                if (Tx_Characteristic == nullptr) {
                        debugf(DEBUG_FLAG_SERIAL, "BLUETOOTH FAILED TO FIND TX CHARACTERISTIC UUID");
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

void bluetoothParse(const uint8_t* data, size_t len) {

        JsonDocument JsonIn;
        DeserializationError err = deserializeJson(JsonIn, data, len);
        if (err) {
            debug("BLUETOOTH RX PARSER JSON ERROR");
            return;
        }
    
        stateold = state;
        state = JsonIn["state"].as<String>();
        if (stateold != state) { statechange = true; }

        wxold = wx;
        wx = JsonIn["wx"].as<float>();
        if (wxold != wx) { wxchange = true; }

        wyold = wy;
        wy = JsonIn["wy"].as<float>();
        if (wyold != wy) { wychange = true; }

        wzold = wz;
        wz = JsonIn["wz"].as<float>();
        if (wzold != wz) { wzchange = true; }

        waold = wa;
        wa = JsonIn["wa"].as<float>();
        if (waold != wa) { wachange = true; }

        debugf(DEBUG_FLAG_SERIAL, "PARSED: state=%s, wx=%.3f, wy=%.3f, wz=%.3f, wa=%.3f", state, wx, wy, wz, wa);

}

void bluetoothSend(const String& type, const String& cmd, const String& cmd_info) {

        JsonDocument JsonOut;
        JsonOut[type] = cmd;                     // {"type":"cmd"}
        char jsonBuf[128];                      // was 64
        size_t jsonLen = serializeJson(JsonOut, jsonBuf, sizeof(jsonBuf));

        if (bluetoothConnect())
        {
                Tx_Characteristic->writeValue(reinterpret_cast<uint8_t*>(jsonBuf), jsonLen);
                debugf(DEBUG_FLAG_SERIAL, "SENT: %s", jsonBuf);
                TFTPrint(MessageField, cmd_info, TFT_COLOR_MSG_NRM);
        }
        else {
                TFTPrint(MessageField, "NO CONNECTION. HOLD!", TFT_COLOR_MSG_ERR);
        }

}

