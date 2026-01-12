#include <Arduino.h>
#include <ESP32Encoder.h>
#include <JC_Button.h>
#include "global/global.h"
#include "input/input.h"


QueueHandle_t inputQueue = nullptr;

ESP32Encoder encoder;
int32_t lastCount = 0;

Button Button0(BUTTON_PIN_0, BUTTON_DEBOUNCE_MS);
Button Button1(BUTTON_PIN_1, BUTTON_DEBOUNCE_MS);
Button Button2(BUTTON_PIN_2, BUTTON_DEBOUNCE_MS);
Button Button3(BUTTON_PIN_3, BUTTON_DEBOUNCE_MS);
Button Button4(BUTTON_PIN_4, BUTTON_DEBOUNCE_MS);
Button Button5(BUTTON_PIN_5, BUTTON_DEBOUNCE_MS);
Button Button6(BUTTON_PIN_6, BUTTON_DEBOUNCE_MS);
Button Button7(BUTTON_PIN_7, BUTTON_DEBOUNCE_MS);
Button Button8(BUTTON_PIN_8, BUTTON_DEBOUNCE_MS);
Button Button9(BUTTON_PIN_9, BUTTON_DEBOUNCE_MS);
Button Button10(BUTTON_PIN_10, BUTTON_DEBOUNCE_MS);
Button Button11(BUTTON_PIN_11, BUTTON_DEBOUNCE_MS);

// --------------------------------------------------------------
// Encoder Setup

void setupEncoder() {
    ESP32Encoder::useInternalWeakPullResistors =  puType::up;
    encoder.attachSingleEdge(ENCODER_PIN_A, ENCODER_PIN_B);
    encoder.clearCount();
    lastCount = 0;
}


void setupButtons() {

    Button0.begin();
    Button1.begin();
    Button2.begin();
    Button3.begin();
    Button4.begin();
    Button5.begin();
    Button6.begin();
    Button7.begin();
    Button8.begin();
    Button9.begin();
    Button10.begin();
    Button11.begin();

}

// --------------------------------------------------------------
// InputTask (Buttons + Encoder)

void InputTask(void* pv) {


    setupEncoder();
    setupButtons();
    uint8_t encoderWait = 0;

    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {


        // ---------- Encoder ----------

        if (encoderWait >= INPUT_POLL_MULTIPLIER_ENCODER)
        {
            int32_t currentCount = encoder.getCount();
            int32_t delta = currentCount - lastCount;
            lastCount = currentCount;

            encoderWait = 0;

            if (delta != 0) {
                InputEvent ev { ENC_MOVE, delta };
                xQueueSend(inputQueue, &ev, 0);
            }
        }
        else {
            encoderWait++;
        }


        // ---------- Buttons ----------

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

        if (Button0.wasPressed()) {
            InputEvent ev { BTN_AXIS_PREV, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button1.wasPressed()) {
            InputEvent ev { BTN_AXIS_NEXT, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button2.wasPressed()) {
            InputEvent ev { BTN_SET_ZERO, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button3.wasPressed()) {
            InputEvent ev { BTN_GOTO_ZERO, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

         if (Button4.wasPressed()) {
            InputEvent ev { BTN_PROBE, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button5.wasPressed()) {
            InputEvent ev { BTN_MENU, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button6.wasPressed()) {
            InputEvent ev { BTN_FACTOR_DOWN, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button7.wasPressed()) {
            InputEvent ev { BTN_FACTOR_UP, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

         if (Button8.wasPressed()) {
            InputEvent ev { BTN_HOME, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button9.wasPressed()) {
            InputEvent ev { BTN_STOP, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button10.wasPressed()) {
            InputEvent ev { BTN_UNLOCK, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        if (Button11.wasPressed()) {
            InputEvent ev { BTN_ENTER, 0 };
            xQueueSend(inputQueue, &ev, 0);
        }

        vTaskDelayUntil(&lastWakeTime, INPUT_POLL_INTERVAL_MS);
}















}


