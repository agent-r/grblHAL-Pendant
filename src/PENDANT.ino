/////////////////////////////////////////
//
//   GRBLHAL PENDANT
//
/////////////////////////////////////////


/////////////////////////////////////////
//
//   BUGS:
//
//      - im Config-Menü werden Encoder-Ticks gesendet
//      - im Config-Menü werden Button-Clicks gesendet (danach)
//      - Sleep funktioniert nicht richtig
//      - 
//      - 
//
//   TODO:
//
//
//
//
//
/////////////////////////////////////////








// PINS
////////////////////////////////////////
// 36   ROT CLK : A
// 39   ROT DT  : B
// 32   KEY AX-
// 33   KEY AX+
// 05   KEY SET0
// 25   KEY GOTO0
// 26   KEY PROBE
// 27   KEY CONFIG
// 14   KEY FEED-
// 13   KEY FEED+
// 16   KEY HOME
// 04   KEY STOP
// 02   KEY RESET
// 15   KEY ENTER
// 21   TFT DC
// 18   TFT SCK
// --   TFT MISO
// 23   TFT MOSI
// 22   TFT CS
// 17   TFT LED
// VCC  TFT RESET
// 34   BATTERY VOLTAGE DIVIDER // NOT USED!


// LOCAL INCLUDES
#include "communication/serial.h"
#include "communication/bluetooth.h"
#include "communication/debug.h"
#include "controls/controls.h"
#include "display/configfunct.h"
#include "display/configmenu.h"
#include "display/display.h"
#include "display/fonts.h"
#include "config.h"
#include "global.h"

// LIBRARIES
#include <soc/soc.h>                // TO DISABLE BROWNOUT DETECTOR) !!
#include <soc/rtc.h>                // TO DISABLE BROWNOUT DETECTOR) !!


///////////////////////////////////////////////////////////////////////////
///////////////////////      SETUP       //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void setup() {

        // DISABLE BROWNOUT DETECTOR
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

        // INITIALIZE SERIAL
        delay(250); 
        serialInit();     
        debug("SERIAL INITIALIZED");

        configLoad();
        bluetoothInit(); debug("BLUETOOTH INITIALIZED");

        TFTInit(); debug("TFT INITIALIZED");
        controlsInit(); debug("CONTROLS INITIALIZED");


        // CONNECT
        bluetoothConnect(); 

        // START TICKERS
        // TftTicker.start();
        if (SleepTime > 0) { SleepTicker.start(); }
        EncoderTicker.start();
        KeypadTicker.start();
        debug("TICKERS STARTED");

        // CHECK IF CONFIG BUTTON IS PRESSED
        if (checkConfig()) { config(); }

}

///////////////////////////////////////////////////////////////////////////
///////////////////////      LOOP        //////////////////////////////////
///////////////////////////////////////////////////////////////////////////


void loop() {

        // TftTicker.update();
        checkEncoder();

        EncoderTicker.update();
        KeypadTicker.update();
        MessageTicker.update();
        SleepTicker.update();

}

