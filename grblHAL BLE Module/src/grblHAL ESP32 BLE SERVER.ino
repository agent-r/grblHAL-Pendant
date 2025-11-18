
/* WORKING UPLOAD SETUP
   USB     ESP-01
   -----------------
   DTR     --
   RX      TX
   TX      RX
   VCC     VCC + CH_PD
   CTS     --
   GND     GND + GPIO-0
 */

// BLE ADDRESS     "0c:b8:15:c3:b1:ea"


// DISABLE BROWNOUT DETECTION
#include <soc/soc.h>
#include <soc/rtc.h>

// #include <WiFi.h> // to disable WiFi
// #include "driver/adc.h" // to disable ADC

// SOFTWARE SERIAL
// #include <SoftwareSerial.h>
// #define SOFTSERIAL_SPEED 38400  // 38400 (?) // 4800 (ok)
// #define SOFTSERIAL_TX 18
// #define SOFTSERIAL_RX 19
// SoftwareSerial SoftSerial;

// SECOND SERIAL (to Teensy/GRBLHAL)
#define SER2_TX 17
#define SER2_RX 16
#define SER2_BAUDRATE 115200

// BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static BLEUUID SERVICE_UUID("825aeb6e-7e1d-4973-9c75-30c042c4770c");

static BLEUUID CHARACTERISTIC_UUID_RX_CMD("24259347-9d86-4c67-a9ae-84f6a7f0c90d");
static BLEUUID CHARACTERISTIC_UUID_RX_GCODE("604fb9a6-3cd8-48b3-a5c6-04e39f3aeccd");
static BLEUUID CHARACTERISTIC_UUID_RX_MSG("8a91b1dc-4574-41d4-a1d0-8d1d7488376a");

static BLEUUID CHARACTERISTIC_UUID_TX_STATE("b52e05ac-8a8a-4880-85c7-bd3e6a32dc0e");
static BLEUUID CHARACTERISTIC_UUID_TX_WX("8261af78-9f01-4525-80ee-30ab576de594");
static BLEUUID CHARACTERISTIC_UUID_TX_WY("47d8deb4-e094-481c-8cf5-a6ad5b20517c");
static BLEUUID CHARACTERISTIC_UUID_TX_WZ("210ea555-b93c-4d3b-8615-fd47a90b4526");
static BLEUUID CHARACTERISTIC_UUID_TX_WA("c3bf023c-45a9-49be-8f66-45600b31287a");


BLEServer *pServer = NULL;
#define bleServerName "GRBLHAL"

BLECharacteristic * pTxStateCharacteristic;
BLECharacteristic * pTxWxCharacteristic;
BLECharacteristic * pTxWyCharacteristic;
BLECharacteristic * pTxWzCharacteristic;
BLECharacteristic * pTxWaCharacteristic;

bool BLEConnected = false;
bool oldBLEConnected = false;

float wx;
float wy;
float wz;
float wa;
String state = "";

String rxCmdString = "";
String rxGCodeString = "";
String rxMsgString = "";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10; // was 100

#define SERIAL_DEBUG
#define SOFT_SERIAL_DEBUG

class MyServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
                BLEConnected = true;
        };

        void onDisconnect(BLEServer* pServer) {
                BLEConnected = false;
        }
};

class RxCmdCallbacks: public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *pCharacteristic) {

                std::string rxValue = pCharacteristic->getValue();
                rxCmdString = "";

                if (rxValue.length() > 0) {
                        for (int i = 0; i < rxValue.length(); i++) {
                                rxCmdString += rxValue[i];
                        }
                }
        }
};

class RxGCodeCallbacks: public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *pCharacteristic) {

                std::string rxValue = pCharacteristic->getValue();
                rxGCodeString = "";

                if (rxValue.length() > 0) {
                        for (int i = 0; i < rxValue.length(); i++) {
                                rxGCodeString += rxValue[i];
                        }
                }
        }
};

class RxMsgCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {

                std::string rxValue = pCharacteristic->getValue();
                rxMsgString = "";

                if (rxValue.length() > 0) {
                        for (int i = 0; i < rxValue.length(); i++) {
                                rxMsgString += rxValue[i];
                        }
                }
    }
};


// JSONs
#include <ArduinoJson.h>
DynamicJsonDocument JsonIn(128);

///////////////////////////////////////////////////////////////////////////
///////////////////////      SETUP       //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void setup() {

        // DISABLE BROWNOUT DETECTOR
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
        // adc_power_off();
        // WiFi.disconnect(true);  // Disconnect from the network
        // WiFi.mode(WIFI_OFF);    // Switch WiFi off


        // Start Serials
        delay(500);
        #ifdef SERIAL_DEBUG
                Serial.begin(115200);
        #endif
        Serial2.begin(SER2_BAUDRATE, SERIAL_8N1, SER2_RX, SER2_TX);
        // SoftSerial.begin(SOFTSERIAL_SPEED, SWSERIAL_8N1, SOFTSERIAL_RX, SOFTSERIAL_TX, false);
        
        SerialMessage("[BLE] starting BLE");

        // Create the BLE Device
        BLEDevice::init(bleServerName);
        
        BLEAddress MyAddress = BLEDevice::getAddress();
        SerialMessage("[BLE] MAC ADDRESS    " + String(MyAddress.toString().c_str()));
        // SerialMessage();

        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());

        // Create the BLE Service
        BLEService *pService = pServer->createService(SERVICE_UUID, 30);

        // Create BLE Characteristics and Create a BLE Descriptor
        // RX
        BLECharacteristic * pRxCmdCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_CMD, BLECharacteristic::PROPERTY_WRITE);
        pRxCmdCharacteristic->addDescriptor(new BLE2902());
        pRxCmdCharacteristic->setCallbacks(new RxCmdCallbacks());
        BLECharacteristic * pRxGCodeCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_GCODE, BLECharacteristic::PROPERTY_WRITE);
        pRxGCodeCharacteristic->addDescriptor(new BLE2902());
        pRxGCodeCharacteristic->setCallbacks(new RxGCodeCallbacks());
        BLECharacteristic * pRxMsgCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX_MSG, BLECharacteristic::PROPERTY_WRITE);
        pRxMsgCharacteristic->addDescriptor(new BLE2902());
        pRxMsgCharacteristic->setCallbacks(new RxMsgCallbacks());

        // TX
        pTxStateCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX_STATE, BLECharacteristic::PROPERTY_NOTIFY);
        pTxStateCharacteristic->addDescriptor(new BLE2902());
        pTxWxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX_WX, BLECharacteristic::PROPERTY_NOTIFY);
        pTxWxCharacteristic->addDescriptor(new BLE2902());
        pTxWyCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX_WY, BLECharacteristic::PROPERTY_NOTIFY);
        pTxWyCharacteristic->addDescriptor(new BLE2902());
        pTxWzCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX_WZ, BLECharacteristic::PROPERTY_NOTIFY);
        pTxWzCharacteristic->addDescriptor(new BLE2902());
        pTxWaCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX_WA, BLECharacteristic::PROPERTY_NOTIFY);
        pTxWaCharacteristic->addDescriptor(new BLE2902());

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->start();
        SerialMessage("[BLE] start advertising");

}

void loop() {

        if ((millis() - lastTime) > timerDelay) {

                if (BLEConnected) {
                        // SUCCESS !!!
                        readAndSendSerial();
                }

                // disconnecting
                if (!BLEConnected && oldBLEConnected) {
                        delay(500); // give the bluetooth stack the chance to get things ready
                        pServer->startAdvertising(); // restart advertising

                        SerialMessage("[BLE] client disconnected");
                        SerialMessage("[BLE] start advertising");

                        oldBLEConnected = BLEConnected;

                        readAwaySerial();
                }
                // connecting
                if (BLEConnected && !oldBLEConnected) {
                                // do stuff here on connecting
                        oldBLEConnected = BLEConnected;

                        SerialMessage("[BLE] client connected");

                        readAwaySerial();
                }

        lastTime = millis();
        }
}

void readAndSendSerial() {

        // get from GRBLHAL & send to PENDANT
        static char in_buffer[128];
        static int i = 0;
        char in_buffer_char;

        while (Serial2.available()) {

                in_buffer_char = Serial2.read();

                if (in_buffer_char == '{') {
                        i = 0;
                        in_buffer[i] = in_buffer_char;
                }
                else if (in_buffer_char == '}') {
                        i++;
                        in_buffer[i] = in_buffer_char;
                        in_buffer[i+1] = '\0';
                        i = 0;
                        if (in_buffer[0] == '{') {
                                
                                DeserializationError error = deserializeJson(JsonIn, in_buffer);

                                if (error) {
                                        // if (SERIAL_DEBUG) { Serial.println(error.f_str()); }
                                        SerialMessage("[BLE] JSON deserialisation error");
                                }
                                else {
                                        //Set RX Characteristic value and notify connected client
                                        if (JsonIn.containsKey("wx")) {
                                                float wx = JsonIn["wx"];
                                                String wxstr = String(wx) + "\n";
                                                pTxWxCharacteristic->setValue(wxstr.c_str());
                                                pTxWxCharacteristic->notify();
                                                #ifdef SERIAL_DEBUG
                                                        Serial.print("   WX: "); Serial.print(wx);
                                                #endif
                                        }
                                        if (JsonIn.containsKey("wy")) {
                                                float wy = JsonIn["wy"];
                                                String wystr = String(wy) + "\n";
                                                pTxWyCharacteristic->setValue(wystr.c_str());
                                                pTxWyCharacteristic->notify();
                                                #ifdef SERIAL_DEBUG
                                                        Serial.print("   WY: "); Serial.print(wy);
                                                #endif
                                        }
                                        if (JsonIn.containsKey("wz")) {
                                                float wz = JsonIn["wz"];
                                                String wzstr = String(wz) + "\n";
                                                pTxWzCharacteristic->setValue(wzstr.c_str());
                                                pTxWzCharacteristic->notify();
                                                #ifdef SERIAL_DEBUG
                                                        Serial.print("   WZ: "); Serial.print(wz);
                                                #endif
                                        }
                                        if (JsonIn.containsKey("wa")) {
                                                float wa = JsonIn["wa"];
                                                String wastr = String(wa) + "\n";
                                                pTxWaCharacteristic->setValue(wastr.c_str());
                                                pTxWaCharacteristic->notify();
                                                #ifdef SERIAL_DEBUG
                                                        Serial.print("   WA: "); Serial.print(wa);
                                                #endif
                                        }
                                        if (JsonIn.containsKey("state")) {
                                                const char * statechar = JsonIn["state"];
                                                String statestr = String(statechar) + "\n";
                                                pTxStateCharacteristic->setValue(statestr.c_str());
                                                pTxStateCharacteristic->notify();
                                                #ifdef SERIAL_DEBUG
                                                        Serial.print("   STATE: "); Serial.print(statestr);
                                                #endif
                                        }
                                        #ifdef SERIAL_DEBUG
                                                Serial.println();
                                        #endif
                                }

                        }
                }
                else {
                        if (i < (sizeof(in_buffer)-3)) {
                                i++;
                                in_buffer[i] = in_buffer_char;
                        }
                }
        }

        // Serial2.println("{\"OK\"}");

        //                         
        
        if (rxCmdString != "") {
                #ifdef SERIAL_DEBUG
                        Serial.println("{\"cmd\":\"" + rxCmdString + "\"}");
                #endif
                Serial2.println("{\"cmd\":\"" + rxCmdString + "\"}");
                rxCmdString = "";
        }

        if (rxGCodeString != "") {
                #ifdef SERIAL_DEBUG
                        Serial.println("{\"gcode\":\"" + rxGCodeString + "\"}");
                #endif
                Serial2.println("{\"gcode\":\"" + rxGCodeString + "\"}");
                rxGCodeString = "";
        }

        if (rxMsgString != "") {
                #ifdef SERIAL_DEBUG
                        Serial.println("{\"msg\":\"" + rxMsgString + "\"}");
                #endif
                Serial2.println("{\"msg\":\"" + rxMsgString + "\"}");
                rxMsgString = "";
        }

}


void readAwaySerial() {
        while(Serial2.available()) {
                Serial2.read();
        }
}


void SerialMessage(String Message) {
        #ifdef SOFT_SERIAL_DEBUG
                Serial2.println("{\"msg\":\"" + Message + "\"}");
        #endif
        #ifdef SERIAL_DEBUG
                Serial.println(Message);
        #endif
}
