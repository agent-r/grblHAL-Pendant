//
// D0  ROT CLK
// A0  KEY SIG
// D3  ROT DT
// D4  TFT DC
// D5  TFT SCK
// D6  TFT MISO
// D7  TFT MOSI
// D8  TFT CS
// 10  ROT SW !!

#include <ESP8266WiFi.h> // to turn off!

#include <Button2.h>
#include <REncoder.h>
#define ROTARY_PIN_CLK D0
#define ROTARY_PIN_DT D3
#define BUTTON_PIN 10
#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder

REncoder rEncoder(ROTARY_PIN_CLK /* CLK Pin*/, ROTARY_PIN_DT /* DT Pin */, BUTTON_PIN /* SW Pin */);
int RPos = 0;

// ESPRotary r = ESPRotary(ROTARY_PIN_CLK, ROTARY_PIN_DT, CLICKS_PER_STEP);
// Button2 b = Button2(BUTTON_PIN);

#define KEYPAD_PIN A0

#include <ArduinoJson.h>

StaticJsonDocument<128> JsonIn;
StaticJsonDocument<64> JsonOut;

int lasttouched = 0;
int currtouched = 0;

#include <Ticker.h>
void TFTUpdate();
Ticker TFTTicker(TFTUpdate, 200);

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_I2CDevice.h>
#define TFT_DC D4
#define TFT_CS D8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
int tftRotation = 0;


/////////////////////////////////////////////////////////////////

uint16_t activeAxis = 0;
String AxisName[4] = {"", "X", "Y", "Z"};

float wx = 1;
float wy = 1;
float wz = 1;
String state = "";
float wxold = 1;
float wyold = 1;
float wzold = 1;
String stateold = "";
bool wxchange = true;
bool wychange = true;
bool wzchange = true;
bool statechange = true;
bool factorchange = true;
bool axischange = true;
uint16_t activeFactor = 2;
float factor[5] = {0.01, 0.1, 1, 10, 100};
String strFactor[5] = {"0.01", "0.10", "1.00", "10.0", "100 "};
String LastSentCommand = "";
String StatusMessage = "";
int StatusMessageCounter = 0;
int MessageTime = 15;

/////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////

void setup() {

    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();

    Serial.setTimeout(50); // 200 works good - less ???
    Serial.begin(128000);
    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
        delay(50);
    }


    // rEncoder.setMinEncoderPosition(-1);
    // rEncoder.setMaxEncoderPosition(1);
    // r.setChangedHandler(rotate);
    // b.setTapHandler(click);

    // stateTicker.start();
    TFTTicker.start();

    tft.begin();
    tft.setRotation(tftRotation);
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 00);
    tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);

    tft.println("Starting Pendant ...");
    tft.println("");

    delay(500); // ????

    tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
}


void loop() {

    getRotary();
    getButton();
    getState();

    TFTTicker.update();
}


///////////////////////// ROT ENCODER ////////////////////////////////////////

void getRotary() {
    // REncoder::Event encoderEvent = rEncoder.reState();

    switch (rEncoder.reState()) {
    case REncoder::Event::REncoder_Event_Rotate_CW:
        if( digitalRead(ROTARY_PIN_CLK) == HIGH) {
            if (activeAxis != 0) {
                sendCmd("G", "G91G0" + AxisName[activeAxis] + (factor[activeFactor] * (-1 * rEncoder.getPosition())));
                sendCmd("G", "G90");
            }
            else {PrintStatus("ERROR: No Axis!", ILI9341_WHITE);}
        }
        rEncoder.setPosition(0);
        break;
    case REncoder::Event::REncoder_Event_Rotate_CCW:
        if( digitalRead(ROTARY_PIN_CLK) == HIGH) {
            if (activeAxis != 0) {
                sendCmd("G", "G91G0" + AxisName[activeAxis] + (factor[activeFactor] * (-1 * rEncoder.getPosition())));
                sendCmd("G", "G90");
            }
            else {PrintStatus("ERROR: No Axis!", ILI9341_WHITE);}
        }
        rEncoder.setPosition(0);
        break;
    case REncoder::Event::REncoder_Event_Switch_Pushed:
        if (activeAxis != 0) {
            sendCmd("G", "G10L20P1" + AxisName[activeAxis] + "0");
        }
        else { PrintStatus("ERROR: No Axis!", ILI9341_WHITE); }
        break;
    case REncoder::Event::REncoder_Event_None:
        break;
    }

}



/////////////////////////////////////////////////////////////////

void getButton() {

    currtouched = readButton();
    delay(10);
    if (currtouched != readButton()) {return;}

    if (currtouched != lasttouched) {
        switch (currtouched) {
        case -1:
            break;
        case 0:
            setX();
            break;
        case 1:
            setY();
            break;
        case 2:
            setZ();
            break;
        case 3:
            homeX();
            break;
        case 4:
            homeY();
            break;
        case 5:
            homeZ();
            break;
        case 6:
            decreaseFactor();
            break;
        case 7:
            increaseFactor();
            break;
        case 8:
            homeAll();
            break;
        case 9:
            run();
            break;
        case 10:
            stop();
            break;
        case 11:
            reset_unlock();
            break;
        default:
            // Statement(s)
            break;
        }
    }
    lasttouched = currtouched;
}

int readButton() {
    int analogValue = analogRead(KEYPAD_PIN);
    if (analogValue < 450) {
        return(-1);
    } else if (analogValue < 520) {
        return(0);
    } else if (analogValue < 540) {
        return(1);
    } else if (analogValue < 570) {
        return(2);
    } else if (analogValue < 600) {
        return(3);
    } else if (analogValue < 650) {
        return(4);
    } else if (analogValue < 700) {
        return(5);
    } else if (analogValue < 750) {
        return(6);
    } else if (analogValue < 800) {
        return(7);
    } else if (analogValue < 850) {
        return(8);
    } else if (analogValue < 930) {
        return(9);
    } else if (analogValue < 1000) {
        return(10);
    } else {
        return(11);
    }
}


////////////////////////

void getState() {

    if (Serial.available() > 0) {
        deserializeJson(JsonIn, Serial);
        wz = JsonIn["wz"]; // 0.1
        wy = JsonIn["wy"]; // 0.1
        wx = JsonIn["wx"]; // 0.1
        const char* statechar = JsonIn["state"];
        state = String(statechar);
        if (wx != wxold) { wxchange = true; wxold = wx; }
        if (wy != wyold) { wychange = true; wyold = wy; }
        if (wz != wzold) { wzchange = true; wzold = wz; }
        if (state != stateold) { statechange = true; stateold = state; }
    }

}

void sendCmd(String type, String cmd) {
    JsonOut.clear();
    JsonOut[type] = cmd;
    serializeJson(JsonOut, Serial);
    Serial.print(";");

    if ((cmd == "G90") || (cmd == "UNLOCK")) {
        LastSentCommand = LastSentCommand + " " + cmd;
    }
    else {
        LastSentCommand = cmd;
    }
    PrintStatus(LastSentCommand, ILI9341_WHITE);
}


void TFTUpdate() {

    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE);

    if (wxchange || axischange) {
        char strwx[7];
        dtostrf(wx,6,2,strwx);
        tft.setCursor(10, 20);
        tft.fillRect(0, 0, 240, 55, ILI9341_BLACK);
        if (activeAxis == 1) {tft.print(">");}
        else {tft.print(" ");}
        tft.print(" X:");
        tft.print(strwx);
        wxchange = false;
    }
    if (wychange || axischange) {
        char strwy[7];
        dtostrf(wy,6,2,strwy);
        tft.setCursor(10, 55);
        tft.fillRect(0, 55, 240, 35, ILI9341_BLACK);
        if (activeAxis == 2) {tft.print(">");}
        else {tft.print(" ");}
        tft.print(" Y:");
        tft.println(strwy);
        wychange = false;
    }
    if (wzchange || axischange) {
        char strwz[7];
        dtostrf(wz,6,2,strwz);
        tft.setCursor(10, 90);
        tft.fillRect(0, 90, 240, 35, ILI9341_BLACK);
        if (activeAxis == 3) {tft.print(">");}
        else {tft.print(" ");}
        tft.print(" Z:");
        tft.println(strwz);
        wzchange = false;
        axischange = false;
    }
    if (factorchange) {
        tft.setTextColor(ILI9341_DARKGREY);
        tft.setCursor(10, 150);
        tft.fillRect(0, 150, 240, 35, ILI9341_BLACK);
        tft.print("  F: ");
        tft.print(strFactor[activeFactor]);
        factorchange = false;
    }
    if (statechange) {
        tft.fillRect(0, 220, 240, 39, ILI9341_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_YELLOW);
        tft.setCursor(20, 220);
        tft.print(state);
        statechange = false;
    }

    if (StatusMessageCounter == MessageTime) { tft.fillRect(0, 260, 240, 39, ILI9341_BLACK); }
    StatusMessageCounter++;

}

void PrintStatus(String status, int color) {
    tft.fillRect(0, 260, 240, 39, ILI9341_BLACK);
    if(status != "") {
        tft.setTextSize(2);
        tft.setTextColor(color);
        tft.setCursor(20, 260);
        tft.print(status);
    }
    StatusMessageCounter = 0;
}

void setX() {
    activeAxis = 1;
    axischange = true;
}
void setY() {
    activeAxis = 2;
    axischange = true;
}
void setZ() {
    activeAxis = 3;
    axischange = true;
}
void homeX() {
    sendCmd("G", "G0X0");
}
void homeY() {
    sendCmd("G", "G0Y0");
}
void homeZ() {
    sendCmd("G", "G0Z0");
}
void homeAll() {
    PrintStatus("CONFIRM HOME ??", ILI9341_RED);
    for (int i = 0; i < 50; i++) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            sendCmd("C", "HOME");
            while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
            return;
        }
        delay(50);
    }
    PrintStatus("", ILI9341_BLACK);
}
void stop() {
    sendCmd("C", "STOP");
}
void reset_unlock() {
    sendCmd("C", "RESET");
    sendCmd("C", "UNLOCK");
}
void increaseFactor(){
    if (activeFactor >= 4) {activeFactor = 4;}
    else {activeFactor++;}
    factorchange = true;
}
void decreaseFactor(){
    if (activeFactor <= 0) {activeFactor = 0;}
    else {activeFactor--;}
    factorchange = true;
}
void run() {
    PrintStatus("CONFIRM RUN ??", ILI9341_RED);
    for (int i = 0; i < 50; i++) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            sendCmd("C", "RUN");
            while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
            return;
        }
        delay(50);
    }
    PrintStatus("", ILI9341_BLACK);
}
