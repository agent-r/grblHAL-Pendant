
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <NewEncoder.h>             // ROTARY ENCODER
#include <JC_Button.h>              // BUTTON CHECKER
#include <TickTwo.h>                // TICKER

#include "controls.h"
#include "global.h"
#include "communication/bluetooth.h"
#include "display/display.h"
#include "display/configmenu.h"

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


// ENCODER

static QueueHandle_t encoderQueue;

static void ESP_ISR callBack(NewEncoder* encPtr, const volatile NewEncoder::EncoderState* state, void* uPtr) {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xQueueOverwriteFromISR(encoderQueue, (void*)state, &pxHigherPriorityTaskWoken);
    if (pxHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void handleEncoder(void* pvParameters) {
    NewEncoder::EncoderState oldState, newState;

    // Queue für Encoder-Events erstellen
    encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));
    configASSERT(encoderQueue);

    // Encoder initialisieren
    NewEncoder* encoder1 = new NewEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_MIN, ENCODER_MAX, 0, HALF_PULSE); // HALF_PULSE or FULL_PULSE
    if (!encoder1->begin()) {
        delete encoder1;
        vTaskDelete(nullptr);
    }

    // Callback für ISR anhängen
    encoder1->attachCallback(callBack);

    for (;;) {
        // Auf neuen Encoderwert warten
        if (xQueueReceive(encoderQueue, &newState, portMAX_DELAY)) {
            // Delta seit letztem Reset ermitteln und Encoder auf 0 zurücksetzen
            if (encoder1->getAndSet(0, oldState, newState)) {
                EncoderValue = -oldState.currentValue;   // Änderung seit letztem Mal
            }
        }
    }

    vTaskDelete(nullptr);
}


/*
static void ESP_ISR callBack(NewEncoder*encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr) {
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

        xQueueOverwriteFromISR(encoderQueue, (void * )state, &pxHigherPriorityTaskWoken);
        if (pxHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
        }
}

static void handleEncoder(void *pvParameters) {
        NewEncoder::EncoderState currentEncoderstate;
        int16_t currentValue;

        encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));
        if (encoderQueue == nullptr) {
                vTaskDelete(nullptr);
        }

        NewEncoder *encoder1 = new NewEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_MIN, ENCODER_MAX, 0, FULL_PULSE); // HALF_PULSE ??
        if (encoder1 == nullptr) {
                vTaskDelete(nullptr);
        }

        if (!encoder1->begin()) {
                delete encoder1;
                vTaskDelete(nullptr);
        }

        encoder1->getState(currentEncoderstate);
        encoder1->attachCallback(callBack);

        for (;;) {
                xQueueReceive(encoderQueue, &currentEncoderstate, portMAX_DELAY);
                currentValue = currentEncoderstate.currentValue;
                EncoderValue -= currentValue;

                encoder1->newSettings(ENCODER_MIN, ENCODER_MAX, 0, currentEncoderstate);
        }
        vTaskDelete(nullptr);
}
*/

void checkEncoder();
TickTwo EncoderTicker(checkEncoder, (1000 / ENCODER_FPS));

// #define KEYPAD_PIN GPIO_NUM_33       // ONLY ADC1-PINS !!!
void checkKeypad();
TickTwo KeypadTicker(checkKeypad, BUTTON_DEBOUNCE);


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

        // STARTING ENCODER INTERRUPTS:
        BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
        if (!success) {
                printf("Failed to create handleEncoder task. Aborting.\n");
                while (1) {
                        yield();
                }
        }


}

void checkEncoder() {
        if (EncoderChange()) {
                bluetoothSend("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * readEncoder() * AxisDir[activeAxis]) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * readEncoder() * AxisDir[activeAxis]));
                EncoderValue = 0;
                if (SleepTime > 0) { SleepTicker.start(); }
        }
}

bool EncoderChange() {
        return(EncoderValue != 0);
}

int16_t readEncoder() {
        return (EncoderValue);
}

void resetEncoder() {
        EncoderValue = 0;
}


/////////////////////////////    KEYPAD    ////////////////////////////////////


void checkKeypad() {

        uint8_t keypad_current = readKeypad(true);

        // if (keypad_current != keypad_old) {

                #ifdef SERIAL_DEBUG
                        if (keypad_current != 12) { Serial.println("KEYPAD PRESS: " + String(keypad_current)) ;}
                #endif
                // if ((keypad_current != 12) && (SleepTime > 0)) { SleepTicker.start(); }

                switch (keypad_current) {
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
        // }
               
}


int readKeypad(bool buffered) {


        Button0.read();
        Button1.read();
        Button2.read();
        Button3.read();
        Button4.read();
        Button5.read();
        Button6.read();
        Button7.read();
        Button8.read();
        Button9.read();
        Button10.read();
        Button11.read();

        if (Button0.wasReleased()) { return(0); }
        else if (Button1.wasReleased()) { return(1); }
        else if (Button2.wasReleased()) { return(2); }
        else if (Button3.wasReleased()) { return(3); }
        else if (Button4.wasReleased()) { return(4); }
        else if (Button5.wasReleased()) { return(5); }
        else if (Button6.wasReleased()) { return(6); }
        else if (Button7.wasReleased()) { return(7); }
        else if (Button8.wasReleased()) { return(8); }
        else if (Button9.wasReleased()) { return(9); }
        else if (Button10.wasReleased()) { return(10); }
        else if (Button11.wasReleased()) { return(11); }
        else { return(12); }

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


float readBattery() {
        // int batteryValue = analogRead(BATTERY_PIN);
        // float batteryVoltage = (batteryValue * BATTERY_FACTOR);
        // return(batteryVoltage);
}

int percentageBattery(const float Voltage) {
        // int Percentage = ((Voltage - BATTERY_LOW) * 100) / (BATTERY_HIGH - BATTERY_LOW);
        // return(Percentage);
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
