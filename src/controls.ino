
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#define ENCODER_MIN -1000
#define ENCODER_MAX 1000
bool EncoderToggle = true;

void handleEncoder(void *pvParameters) {
        NewEncoder::EncoderState currentEncoderstate;
        int16_t currentValue;

        encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));
        if (encoderQueue == nullptr) {
                // printf("Failed to create encoderQueue. Aborting\n");
                vTaskDelete(nullptr);
        }

        // Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
        // Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD
        NewEncoder *encoder1 = new NewEncoder(CORE_INT36_PIN, CORE_INT39_PIN, ENCODER_MIN, ENCODER_MAX, 0, HALF_PULSE);
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

                if (EncoderToggle) {
                        EncoderValue -= currentValue;
                }
                EncoderToggle = !EncoderToggle;
                encoder1->newSettings(ENCODER_MIN, ENCODER_MAX, 0, currentEncoderstate);
        }
        vTaskDelete(nullptr);
}

void ESP_ISR callBack(NewEncoder*encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr) {
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

        xQueueOverwriteFromISR(encoderQueue, (void * )state, &pxHigherPriorityTaskWoken);
        if (pxHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
        }
}


void checkEncoder() {
        if (EncoderChange()) {
                sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * readEncoder() * AxisDir[activeAxis]) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * readEncoder() * AxisDir[activeAxis]));
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

        if (keypad_current != -2) { keypad_old = keypad_current; }
        keypad_current = readKeypad(true);

        // if (keypad_current != keypad_old) {
        if (keypad_current != keypad_old) {

                #ifdef SERIAL_DEBUG
                        Serial.println("KEYPAD PRESS: " + String(keypad_current));
                #endif
                // if ((keypad_current != 12) && (SleepTime > 0)) { SleepTicker.start(); }

                switch (keypad_current) {
                case 13: break;                  // error when button pressed !
                case 12: break;                  // no button pressed !
                case 0: SleepTicker.start(); enter(); break;                      // setX()
                case 1: SleepTicker.start(); unlock(); break;                      // setY
                case 2: SleepTicker.start(); stop(); break;                      // setZ
                case 3: SleepTicker.start(); homeAll(); break;                  // gotoZero()
                case 4: SleepTicker.start(); increaseFactor(); break;                    // probeZ()
                case 5: SleepTicker.start(); decreaseFactor(); break;                    // config()
                case 6: SleepTicker.start(); config(); break;            // decreaseFactor()
                case 7: SleepTicker.start(); probeZ(); break;            // increaseFactor()
                case 8: SleepTicker.start(); gotoZero(); break;                   // homeAll()
                case 9: SleepTicker.start(); setZero(); break;                       // run()
                case 10: SleepTicker.start(); increaseAxis(); break;                     // stop()
                case 11: SleepTicker.start(); decreaseAxis(); break;                   // unlock()
                }
        }
}


int readKeypad(bool buffered) {

        int analogValue = analogRead(KEYPAD_PIN);

        keypad_buffer[0]++;
        if (keypad_buffer[0] > 3) {
                keypad_buffer[0] = 1;
        }

        if      (analogValue < 450 ) { keypad_buffer[keypad_buffer[0]] = 12; }
        else if (analogValue < 1800) { keypad_buffer[keypad_buffer[0]] =  0; }
        else if (analogValue < 1890) { keypad_buffer[keypad_buffer[0]] =  1; }     // was 1900
        else if (analogValue < 1990) { keypad_buffer[keypad_buffer[0]] =  2; }     // was 2000
        else if (analogValue < 2150) { keypad_buffer[keypad_buffer[0]] =  3; }
        else if (analogValue < 2300) { keypad_buffer[keypad_buffer[0]] =  4; }
        else if (analogValue < 2450) { keypad_buffer[keypad_buffer[0]] =  5; }
        else if (analogValue < 2600) { keypad_buffer[keypad_buffer[0]] =  6; }
        else if (analogValue < 2800) { keypad_buffer[keypad_buffer[0]] =  7; }
        else if (analogValue < 3100) { keypad_buffer[keypad_buffer[0]] =  8; }
        else if (analogValue < 3400) { keypad_buffer[keypad_buffer[0]] =  9; }
        else if (analogValue < 3850) { keypad_buffer[keypad_buffer[0]] = 10; }
        else {                         keypad_buffer[keypad_buffer[0]] = 11; }

        if (buffered) {
                if ((keypad_buffer[1] == keypad_buffer[2]) && (keypad_buffer[1] == keypad_buffer[3])) {
                        return(keypad_buffer[1]);
                }
                else {
                        return(13); // very wrong!
                }
        }
        else {
                return ( keypad_buffer[keypad_buffer[0]] );
        }
}


bool checkEnter() {
        if (readKeypad(false) == 0) {
                while (readKeypad(false) != 12) { delay(KEYPAD_DEBOUNCE); }
                return(true);
        }
        else {
                return(false);
        }
}

bool checkEnterConfirm() {
        for (int i = 0; i < 50; i++) {
                if (checkEnter()) { return(true); }
#ifndef BLE_ONLY
                StateTicker.update();
#endif
                TftTicker.update();
                delay(KEYPAD_DEBOUNCE);
        }
        return(false);
}

bool checkConfig() {
        if (readKeypad(false) == 6) {
                while (readKeypad(false) != 12) { delay(KEYPAD_DEBOUNCE); }
                return(true);
        }
        else {
                return(false);
        }
}

////////////////////////    BATTERY    ////////////////////////

/*
   void checkBattery() {
        debug("Checking Battery...");

        // WHY IS THIS CAUSING TROUBLE ????
        //   float battery_state = readBattery();
        //   int battery_percentage = percentageBattery(battery_state);
        //   String batteryMessage = "Battery: " + String(battery_state) + "V / " + String(battery_percentage) + "%";
        //   if (battery_percentage < 25) { TFTPrint(MessageField, batteryMessage, TFT_COLOR_MSG_ERR); }
        //   else { TFTPrint(MessageField, batteryMessage, TFT_COLOR_MSG_NRM); }
   }
 */

float readBattery() {
        int batteryValue = analogRead(BATTERY_PIN);
        float batteryVoltage = (batteryValue * BATTERY_FACTOR);
        return(batteryVoltage);
}

int percentageBattery(const float Voltage) {
        int Percentage = ((Voltage - BATTERY_LOW) * 100) / (BATTERY_HIGH - BATTERY_LOW);
        return(Percentage);
}
