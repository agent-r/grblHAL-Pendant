
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <ESP32Encoder.h>
#include <JC_Button.h>              // BUTTON CHECKER
#include <TickTwo.h>                // TICKER
#include <ArduinoJson.h>

#include "controls.h"
#include "global.h"
#include "communication/bluetooth.h"
#include "display/display.h"
#include "display/configmenu.h"
#include "communication/debug.h"

Button Button0(BUTTON_0_PIN, BUTTON_DEBOUNCE);
Button Button1(BUTTON_1_PIN, BUTTON_DEBOUNCE);
Button Button2(BUTTON_2_PIN, BUTTON_DEBOUNCE);
Button Button3(BUTTON_3_PIN, BUTTON_DEBOUNCE);
Button Button4(BUTTON_4_PIN, BUTTON_DEBOUNCE);
Button Button5(BUTTON_5_PIN, BUTTON_DEBOUNCE);
Button Button6(BUTTON_6_PIN, BUTTON_DEBOUNCE);
Button Button7(BUTTON_7_PIN, BUTTON_DEBOUNCE);
Button Button8(BUTTON_8_PIN, BUTTON_DEBOUNCE);
Button Button9(BUTTON_9_PIN, BUTTON_DEBOUNCE);
Button Button10(BUTTON_10_PIN, BUTTON_DEBOUNCE);
Button Button11(BUTTON_11_PIN, BUTTON_DEBOUNCE);

struct KeypadEvent { uint8_t button; };
struct BleEvent { uint8_t data[BLE_MAX_PACKET]; size_t len; };

QueueHandle_t keypadQueue;
QueueHandle_t bleQueue = nullptr; // ist bereits global definiert

// #define KEYPAD_PIN GPIO_NUM_33       // ONLY ADC1-PINS !!!
void checkKeypad();
TickTwo KeypadTicker(checkKeypad, BUTTON_TICKER_TIME);
void handleKeypadEvent(uint8_t pressedButton);

ESP32Encoder encoder;
void sendEncoder();
TickTwo EncoderTicker(sendEncoder, ENCODER_TICKER_TIME);


/////////////////// CONTROLS TASK AND INIT /////////////////////////

void controlTask(void* pvParameters) {

        KeypadEvent keyEvt;
        BleEvent bleEvt;
    
        for (;;) {
    
            // Keypad Events verarbeiten
            while (xQueueReceive(keypadQueue, &keyEvt, 0) == pdTRUE) {
                handleKeypadEvent(keyEvt.button);
            }
    
            // BLE Events verarbeiten
            while (xQueueReceive(bleQueue, &bleEvt, 0) == pdTRUE) {
                bluetoothParse(bleEvt.data, bleEvt.len);
            }
    
            // checkEncoder();

            // TFT aktualisieren
            TFTUpdate();

            vTaskDelay(1); // Task kurz yielden
        }
    }


void controlsInit() {

        // BUTTONS
        Button0.begin();              // initialize the button object
        Button1.begin();              // initialize the button object
        Button2.begin();              // initialize the button object
        Button3.begin();              // initialize the button object
        Button4.begin();              // initialize the button object
        Button5.begin();              // initialize the button object
        Button6.begin();              // initialize the button object
        Button7.begin();              // initialize the button object
        Button8.begin();              // initialize the button object
        Button9.begin();              // initialize the button object
        Button10.begin();              // initialize the button object
        Button11.begin();              // initialize the button object

        // Queues erstellen (WICHTIG: VOR Task-Start)
        keypadQueue  = xQueueCreate(8, sizeof(KeypadEvent));
        bleQueue     = xQueueCreate(8, sizeof(BleEvent));

        configASSERT(keypadQueue);
        configASSERT(bleQueue);

        // ZENTRALER CONTROLLER TASK
        xTaskCreatePinnedToCore( controlTask, "Controller", 4096, nullptr, 2, nullptr, 1 );

        ESP32Encoder::useInternalWeakPullResistors = puType::up;
        encoder.attachSingleEdge (ENCODER_PIN_A, ENCODER_PIN_B);
        encoder.setCount(0);

}


/////////////////////////////    ENCODER    ////////////////////////////////////

void checkEncoder() {

        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&mux);
                encoderValue = encoder.getCount();
        portEXIT_CRITICAL(&mux);

}


void sendEncoder() {

        int16_t sendValue = encoderValue;
        encoder.setCount(0);
        
        if (sendValue == 0) { return; }              // nichts zu tun

        char gcode[64];
        char msg[20];
        float distance = factor[activeFactor] * sendValue * AxisDir[activeAxis];


        int gcodelength = snprintf(gcode, sizeof(gcode), "$J=G91%s%.3fF%d", AxisName[activeAxis], distance, JogSpeed[activeAxis]);
        int msglength = snprintf(msg, sizeof(msg), "JOG %s%.3f", AxisName[activeAxis], distance);

        // Falls das Buffer zu klein war → n >= sizeof(gcode)
        if (gcodelength < 0 || (size_t)gcodelength >= sizeof(gcode)) { debug("GCODE BUFFER OVERFLOW"); return; }
        if (msglength < 0 || (size_t)msglength >= sizeof(msg)) { debug("MESSAGE BUFFER OVERFLOW"); return; }
    
        // debugf(DEBUG_FLAG_SERIAL, "ENCODER: %d", sendValue);
        bluetoothSend("gcode", String(gcode), String(msg));

        if (SleepTime > 0) { SleepTicker.start(); }

}

void resetEncoder() {
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&mux);
                encoder.clearCount();
        portEXIT_CRITICAL(&mux);
}

int16_t readEncoder() {
        int16_t Value = encoderValue;
        resetEncoder(); 
        return(Value);
}


/////////////////////////////    KEYPAD    ////////////////////////////////////

void checkKeypad() {
        uint8_t btn = readKeypad();
        if (btn != 12) { // 12 = keine Taste
            KeypadEvent evt;
            evt.button = btn;
            xQueueSend(keypadQueue, &evt, 0); // Task-safe
        }
    }


uint8_t readKeypad() {

        Button0.read(); if (Button0.wasReleased()) { return(0); }
        Button1.read(); if (Button1.wasReleased()) { return(1); }
        Button2.read(); if (Button2.wasReleased()) { return(2); }
        Button3.read(); if (Button3.wasReleased()) { return(3); }
        Button4.read(); if (Button4.wasReleased()) { return(4); }
        Button5.read(); if (Button5.wasReleased()) { return(5); }
        Button6.read(); if (Button6.wasReleased()) { return(6); }
        Button7.read(); if (Button7.wasReleased()) { return(7); }
        Button8.read(); if (Button8.wasReleased()) { return(8); }
        Button9.read(); if (Button9.wasReleased()) { return(9); }
        Button10.read(); if (Button10.wasReleased()) { return(10); }
        Button11.read(); if (Button11.wasReleased()) { return(11); }
        return(12); // (else)

}

void handleKeypadEvent(uint8_t pressedButton) {

        if (pressedButton != 12) { 
                // debugf(DEBUG_FLAG_SERIAL, "BUTTON PRESSED: %d", pressedButton) ;
        }

        if (SleepTime > 0) { SleepTicker.start(); }

        switch (pressedButton) {
        case 12: break;                  // no button pressed !
        case 0: decreaseAxis(); break;                      // setX()
        case 1: increaseAxis(); break;                      // setY
        case 2: setZero(); break;                      // setZ
        case 3: gotoZero(); break;                  // gotoZero()
        case 4: probeZ(); break;                    // probeZ()
        case 5: config(); break;                    // config()
        case 6: decreaseFactor(); break;            // decreaseFactor()
        case 7: increaseFactor(); break;            // increaseFactor()
        case 8: homeAll(); break;                   // homeAll()
        case 9: stop(); break;                       // run()
        case 10: unlock(); break;                     // stop()
        case 11: enter(); break;                   // unlock()
        }

}


bool checkEnter() {
        Button11.read();
        if (Button11.wasReleased()) { return(true); }
        // else if (Button11.isPressed()) { return(true); }
        else { return(false); }
}


bool checkEnterConfirm() {
        for (int i = 0; i < (BUTTON_CONFIRM_ENTER_TIME / BUTTON_DEBOUNCE); i++) {
                if (checkEnter()) { return(true); }
                TFTUpdate();
                delay(BUTTON_DEBOUNCE);
        }
        return(false);
}


bool checkConfig() {

        Button5.read();
        if (Button5.wasReleased())    { debug("CONFIG BUTTON PRESSED - ENTER CONFIG"); return(true); }
        // else if (Button5.isPressed()) { debug("CONFIG BUTTON PRESSED - ENTER CONFIG"); return(true); }
        else { return(false); }

}

void decreaseAxis() {
        if (activeAxis <= 0) { activeAxis = 3; }
        else { activeAxis--; }
        axischange = true;
}

void increaseAxis() {
        if (activeAxis >= 3) { activeAxis = 0; }
        else { activeAxis++; }
        axischange = true;
}

void setZero() {
        //-> P1=G54 P2=G55 P3=G56 P4=G57 P5=G58 P6=G59 ... P0=ActiveWorkspace
        // bluetoothSend("gcode", "G10L20P1" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0");  //-> P1=G54 P2=G55 P3=G56
        bluetoothSend("gcode", "G10L20P0" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0");
}

void gotoZero() {
        bluetoothSend("gcode", "$J=G90" + AxisName[activeAxis] + "0F" + JogSpeed[activeAxis], "GOTO " + AxisName[activeAxis] + "0");
}

void probeZ() {

        bool run_check = false;
        Probe_Alarm = false;

        TFTPrint(MessageField, "Confirm Z PROBE ?", TFT_COLOR_MSG_ERR);
        if (checkEnterConfirm()) {

                bluetoothSend("gcode", "G91", ""); delay(50);
                bluetoothSend("gcode", "G38.2Z" + String(ProbeDepth) + "F" + String(ProbeSpeed), "PROBE: GO DOWN"); delay(100);

                for (int i = 0; i < (ProbeTime * 50); i++) {
                        TFTUpdate();
                        if (state == "Run") { run_check = true; }
                        if ((state == "Idle") && run_check) { break; }

                        if (Probe_Alarm) {
                                break;
                        }
                        delay(20);
                }

                delay(100);
                bluetoothSend("gcode", "G90", "");
                delay(100);
                if (!Probe_Alarm) {
                        // bluetoothSend("gcode", "G10L20Z" + String(ProbeOffset), "PROBE: OK");
                        bluetoothSend("gcode", "G10L20P0Z" + String(ProbeOffset), "PROBE: OK");
                        delay(100);
                        bluetoothSend("gcode", "$J=G91Z" + String(ProbeBackHeight) + "F" + String(JogSpeed[2]), "PROBE: OK");
                }
                else {
                        bluetoothSend("gcode", "$J=G91Z" + String(ProbeDepth* -1) + "F" + String(JogSpeed[2]), "");
                        TFTPrint(MessageField, "PROBE: ALARM", TFT_COLOR_MSG_ERR);
                }
        }
}


void homeAll() {
        TFTPrint(MessageField, "Confirm HOME ?", TFT_COLOR_MSG_ERR);
        if (checkEnterConfirm()) {
                // sendCmd("gcode", "$H", "HOME ALL");
                bluetoothSend("cmd", "HOME", "HOME ALL");
                return;
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}

void stop() {
        bluetoothSend("cmd", "STOP", "STOP");
}

void unlock() {
        bluetoothSend("cmd", "UNLOCK", "UNLOCK");
}
void increaseFactor(){
        if (activeFactor >= 4) {activeFactor = 1;}
        else {activeFactor++;}
        factorchange = true;
}
void decreaseFactor(){
        if (activeFactor <= 0) {activeFactor = 4;}
        else {activeFactor--;}
        factorchange = true;
}

void enter() {
        bluetoothSend("cmd", "START", "START");
}


/////////////////// BATTERY /////////////////////////

float readBattery() {
        // int batteryValue = analogRead(BATTERY_PIN);
        // float batteryVoltage = (batteryValue * BATTERY_FACTOR);
        // return(batteryVoltage);
}

int percentageBattery(const float Voltage) {
        // int Percentage = ((Voltage - BATTERY_LOW) * 100) / (BATTERY_HIGH - BATTERY_LOW);
        // return(Percentage);
}


