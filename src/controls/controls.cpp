
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <NewEncoder.h>             // ROTARY ENCODER
#include <JC_Button.h>              // BUTTON CHECKER
#include <TickTwo.h>                // TICKER
#include <ArduinoJson.h>

#include "controls.h"
#include "global.h"
#include "communication/bluetooth.h"
#include "display/display.h"
#include "display/configmenu.h"
#include "communication/debug.h"

#define ENCODER_MIN -1000
#define ENCODER_MAX 1000


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


struct EncoderEvent { int16_t delta; };
struct KeypadEvent { uint8_t button; };
struct BleEvent { uint8_t data[BLE_MAX_PACKET]; size_t len; };

QueueHandle_t encoderQueue;
QueueHandle_t keypadQueue;
QueueHandle_t bleQueue = nullptr; // ist bereits global definiert

static NewEncoder* encoder1 = nullptr;

static void ESP_ISR encoderCallback( NewEncoder* enc, const volatile NewEncoder::EncoderState* state, void* uPtr ) {
        // statischer letzter Wert – bleibt zwischen ISR-Aufrufen erhalten
        static int16_t lastValue = 0;
        BaseType_t woken = pdFALSE;

        if (state != nullptr) {           // <--- sicherstellen, dass state gültig ist
                int16_t current = state->currentValue;
                int16_t delta = current - lastValue;
                lastValue = current;
            
                if (encoderQueue != nullptr && delta != 0) {
                    EncoderEvent evt{.delta = delta};
                    xQueueSendFromISR(encoderQueue, &evt, &woken);
                }
        }        
    
        if (woken) {
            portYIELD_FROM_ISR();
        }
}

    
static void handleEncoder(void*) {
        
        encoder1 = new NewEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_MIN, ENCODER_MAX, 0, HALF_PULSE);

        if (!encoder1->begin()) {
                delete encoder1;
                encoder1 = nullptr;
                vTaskDelete(nullptr);
            }
    
        encoder1->attachCallback(encoderCallback);
        vTaskDelete(nullptr); // Task endet hier!
    }


// #define KEYPAD_PIN GPIO_NUM_33       // ONLY ADC1-PINS !!!
void checkKeypad();
TickTwo KeypadTicker(checkKeypad, BUTTON_TICKER_TIME);

/////////////////// CONTROLS TASK AND INIT /////////////////////////

void controlTask(void* pvParameters) {

        EncoderEvent encEvt;
        KeypadEvent keyEvt;
        BleEvent bleEvt;
    
        for (;;) {
            // Encoder Events verarbeiten
            while (xQueueReceive(encoderQueue, &encEvt, 0) == pdTRUE) {
                encoderValue += encEvt.delta; // delta addieren
                // hier Bluetooth/GCode generieren, TFT-Update etc.
                handleEncoderEvent(encEvt.delta);

            }
    
            // Keypad Events verarbeiten
            while (xQueueReceive(keypadQueue, &keyEvt, 0) == pdTRUE) {
                handleKeypadEvent(keyEvt.button);
            }
    
            // BLE Events verarbeiten
            while (xQueueReceive(bleQueue, &bleEvt, 0) == pdTRUE) {
                bluetoothParse(bleEvt.data, bleEvt.len);
            }
    
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
        encoderQueue = xQueueCreate(8, sizeof(EncoderEvent));
        keypadQueue  = xQueueCreate(8, sizeof(KeypadEvent));
        bleQueue     = xQueueCreate(8, sizeof(BleEvent));

        configASSERT(encoderQueue);
        configASSERT(keypadQueue);
        configASSERT(bleQueue);

        // Encoder-Setup-Task (lebt nur kurz)
        xTaskCreatePinnedToCore( handleEncoder, "EncoderInit", 2048, nullptr, 3, nullptr, 1 );
        // ZENTRALER CONTROLLER TASK
        xTaskCreatePinnedToCore( controlTask, "Controller", 4096, nullptr, 2, nullptr, 1 );
}


/////////////////////////////    ENCODER    ////////////////////////////////////



void handleEncoderEvent(int16_t encoderDelta) {

        if (encoderDelta == 0) { return; }              // nichts zu tun

        char gcode[64];
        char msg[16];
        
        float distance = factor[activeFactor] * encoderDelta * AxisDir[activeAxis];

        int gcodelength = snprintf(gcode, sizeof(gcode), "$J=G91%s%.3fF%s", AxisName[activeAxis], distance, JogSpeed[activeAxis]);
        int msglength = snprintf(msg, sizeof(msg), "JOG %s%.3f", AxisName[activeAxis], distance);


        // Falls das Buffer zu klein war → n >= sizeof(gcode)
        if (gcodelength < 0 || (size_t)gcodelength >= sizeof(gcode)) { debug("[PENDANT] GCode Buffer overflow"); return; }
        if (msglength < 0 || (size_t)msglength >= sizeof(msg)) { debug("[PENDANT] Msg Buffer overflow"); return; }
    
        // bluetoothSend("gcode", String(gcode), String(msg));
        debug (String(encoderDelta));

        if (SleepTime > 0) { SleepTicker.start(); }

}


void resetEncoder() {
        encoderValue = 0;
}

int16_t readEncoder() {
        return(encoderValue);

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

        #ifdef SERIAL_DEBUG
                if (pressedButton != 12) { Serial.println("KEYPAD PRESS: " + String(pressedButton)) ;}
        #endif

        /*
        switch (pressedButton) {
        // case 12: break;                  // no button pressed !
        case 0: SleepTicker.start(); decreaseAxis(); break;                      // setX()
        case 1: SleepTicker.start(); increaseAxis(); break;                      // setY
        case 2: SleepTicker.start(); setZero(); break;                      // setZ
        case 3: SleepTicker.start(); gotoZero(); break;                  // gotoZero()
        case 4: SleepTicker.start(); probeZ(); break;                    // probeZ()
        case 5: SleepTicker.start(); config(); break;                    // config()
        case 6: SleepTicker.start(); decreaseFactor(); break;            // decreaseFactor()
        case 7: SleepTicker.start(); increaseFactor(); break;            // increaseFactor()
        case 8: SleepTicker.start(); homeAll(); break;                   // homeAll()
        case 9: SleepTicker.start(); stop(); break;                       // run()
        case 10: SleepTicker.start(); unlock(); break;                     // stop()
        case 11: SleepTicker.start(); enter(); break;                   // unlock()
        }
        */
}


bool checkEnter() {

        Button11.read();
        if (Button11.wasReleased()) { return(true); }
        else { return(false); }

}


bool checkEnterConfirm() {
        for (int i = 0; i < 50; i++) {
                if (checkEnter()) { return(true); }
                TftTicker.update();
                delay(BUTTON_DEBOUNCE);
        }
        return(false);
}

bool checkConfig() {

        Button5.read();
        if (Button5.wasReleased()) { return(true); }
        else if (Button5.isPressed()) { return(true); }
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
                        // CAN THIS BE DONE WIHT A "DWELL"-COMMAND (=> WAIT FOR BUFFER EMPTY) ???
                        TftTicker.update();
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


