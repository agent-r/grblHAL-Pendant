#include <Arduino.h>
#include <TFT_eSPI.h>
#include "global/global.h"
#include "display/display.h"
#include "display/display_functions.h"

QueueHandle_t displayQueue = nullptr;
TFT_eSPI tft = TFT_eSPI();

// --------------------------------------------------------------

void DisplaySetup()
{

    tft.begin();
    tft.setRotation(TFT_ROTATION);
    pinMode(TFT_LED_PIN, OUTPUT);
    analogWrite(TFT_LED_PIN, config.TFTBrightness);

    TFTPrepare();

    DisplayEvent ev = {};
    TFTPrint(0, ev.state.x, (ev.state.activeAxisIndex == 0) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
    TFTPrint(1, ev.state.y, (ev.state.activeAxisIndex == 1) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
    TFTPrint(2, ev.state.z, (ev.state.activeAxisIndex == 2) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
    TFTPrint(3, ev.state.a, (ev.state.activeAxisIndex == 3) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
    TFTPrint(4, ev.state.jogFactor, TFT_COLOR_XYZ_INACTIVE);
    TFTPrint(5, ev.state.machineState, TFT_COLOR_STA_NRM);
    TFTPrint(6, ev.state.message, TFT_COLOR_MSG_NRM);

}

void DisplayTask(void *pv)
{

    DisplaySetup();

    DisplayEvent ev = {};

    for (;;)
    {
        if (xQueueReceive(displayQueue, &ev, portMAX_DELAY))
        {
            switch (ev.type)
            {

            case DISP_PREPARE:
                TFTPrepare();
                TFTPrint(0, ev.state.x, (ev.state.activeAxisIndex == 0) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
                TFTPrint(1, ev.state.y, (ev.state.activeAxisIndex == 1) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
                TFTPrint(2, ev.state.z, (ev.state.activeAxisIndex == 2) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
                TFTPrint(3, ev.state.a, (ev.state.activeAxisIndex == 3) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE);
                TFTPrint(4, ev.state.jogFactor, TFT_COLOR_XYZ_INACTIVE);
                TFTPrint(5, ev.state.machineState, TFT_COLOR_STA_NRM);
                TFTPrint(6, ev.state.message, TFT_COLOR_MSG_NRM);
                break;

            case DISP_UPDATE_STATUS:

                if (ev.state.xChanged || ev.state.axisChanged) { TFTPrint(0, ev.state.x, (ev.state.activeAxisIndex == 0) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE); }
                if (ev.state.yChanged || ev.state.axisChanged) { TFTPrint(1, ev.state.y, (ev.state.activeAxisIndex == 1) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE); }
                if (ev.state.zChanged || ev.state.axisChanged) { TFTPrint(2, ev.state.z, (ev.state.activeAxisIndex == 2) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE); }
                if (ev.state.aChanged || ev.state.axisChanged) { TFTPrint(3, ev.state.a, (ev.state.activeAxisIndex == 3) ? TFT_COLOR_XYZ : TFT_COLOR_XYZ_INACTIVE); }
                if (ev.state.jogFactorChanged) { TFTPrint(4, ev.state.jogFactor, TFT_COLOR_XYZ_INACTIVE); }
                if (ev.state.stateChanged) { TFTPrint(5, ev.state.machineState, TFT_COLOR_STA_NRM); }
                if (ev.state.messageChanged) { TFTPrint(6, ev.state.message, TFT_COLOR_MSG_NRM);}
                break;

            case DISP_MENU_PREPARE:
                TFTMenuPrepare();
                break;

            case DISP_MENU_NAV_UPDATE:
                TFTMenuPrint(0, ev.nav.title, TFT_COLOR_CNF_STD);
                for (uint8_t i = 0; i < ev.nav.itemCount; i++)
                {
                    if (i == ev.nav.activeIndex)
                    {
                        char buf[27];
                        sprintf(buf, "> %s", ev.nav.items[i]);
                        TFTMenuPrint(i + 1, buf, TFT_COLOR_CNF_HIL);
                    }
                    else
                    {
                        char buf[27];
                        sprintf(buf, "  %s", ev.nav.items[i]);
                        TFTMenuPrint(i + 1, buf, TFT_COLOR_CNF_STD);
                    }
                }
                break;
                
            case DISP_MENU_ADDRESS_UPDATE:

                TFTMenuPrint(0, ev.bleaddress.title, TFT_COLOR_CNF_STD);
                TFTMenuPrint(2, "", TFT_COLOR_CNF_STD);
                TFTMenuPrintAddress(2, ev.bleaddress.left, ev.bleaddress.mid, ev.bleaddress.right, TFT_COLOR_CNF_STD, TFT_COLOR_CNF_HIL);
                break;

            case DISP_MENU_LINE_UPDATE:

                TFTMenuPrint(0, ev.line.title, TFT_COLOR_CNF_STD);
                char printValue[DISP_MAX_MENU_TEXT];
                strncpy(printValue, ev.line.printLine, DISP_MAX_MENU_TEXT);

                TFTMenuPrint(ev.line.line, printValue, TFT_COLOR_CNF_HIL);
                break;

            case DISP_SLEEP:

                TFTSleep();
                break;
            }
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}