///////////////////////////////////////////////////////////////////////////
///////////////////////       DISPLAY       ///////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display/display_functions.h"
#include "display/display.h"
#include "global/global.h"
#include "display/fonts.h"

static const DisplayFormField fields[7] = {
        { .x = 10, .y=20, .width = 220, .height = 35, .fontSize = LARGE, .xTextOffset = 10, .yTextOffset = 30 },
        { .x = 10, .y=60, .width = 220, .height = 35, .fontSize = LARGE, .xTextOffset = 10, .yTextOffset = 30 },
        { .x = 10, .y=100, .width = 220, .height = 35, .fontSize = LARGE, .xTextOffset = 10, .yTextOffset = 30 },
        { .x = 10, .y=140, .width = 220, .height = 35, .fontSize = LARGE, .xTextOffset = 10, .yTextOffset = 30 },
        { .x = 10, .y=190, .width = 220, .height = 35, .fontSize = LARGE, .xTextOffset = 10, .yTextOffset = 30 },
        { .x = 10, .y=245, .width = 220, .height = 28, .fontSize = SMALL, .xTextOffset = 10, .yTextOffset = 22 },
        { .x = 10, .y=278, .width = 220, .height = 28, .fontSize = SMALL, .xTextOffset = 10, .yTextOffset = 22 },
};

static const DisplayLine line = { .x = 0, .y = 234, .width = 1, .color = TFT_COLOR_FRM_LIN };


// --------------------------------------

void TFTPrint(const byte Aim, String Content, const int Color)
{
        Content = Content.substring(0, 20);
        tft.fillRect(fields[Aim].x, fields[Aim].y, fields[Aim].width, fields[Aim].height, TFT_COLOR_FRM_BGR);
        tft.setFreeFont(fields[Aim].fontSize ? &TFT_FONT_SMALL : &TFT_FONT_LARGE);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(fields[Aim].x + fields[Aim].xTextOffset, fields[Aim].y + fields[Aim].yTextOffset);
        tft.print(Content);
}


void TFTPrepare()
{
        tft.fillRect(0, 0, 240, 320, TFT_COLOR_BGR);
        for (int i = 0; i <= 6; i++)
        {
                tft.drawRect(fields[i].x - 1, fields[i].y - 1, fields[i].width + 2, fields[i].height+ 2, TFT_COLOR_FRM_LIN);
        }
        tft.drawFastHLine(line.x, line.y, line.width, line.color);
        TFTClear();
}

void TFTClear()
{
        for (int i = 0; i <= 6; i++)
        {
                tft.fillRect(fields[i].x, fields[i].y, fields[i].width, fields[i].height, TFT_COLOR_FRM_BGR);
        }
}

void TFTMessage()
{
        tft.fillRect(fields[6].x, fields[6].y, fields[6].width, fields[6].height, TFT_COLOR_FRM_BGR);
}

void TFTSleep() 
{
        analogWrite(TFT_LED_PIN, 0);
        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(ILI9341_SLPIN);
}

//////////////////////////////////////////// CONFIG MENU /////////////////////////////////////////

const DisplayMenuForm configForms[2] = {
    { .x=10, .y=15, .width=220, .height=35 },
    { .x=10, .y=60, .width=220, .height=250 }
};


const DisplayMenuField configFields[12] = {
    { .x=10, .y=17, .width=220, .height=33, .fontSize = LARGE, .xTextOffset = 7, .yTextOffset = 28 },
    { .x=10, .y=62, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=84, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=106, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=128, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=150, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=172, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=194, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=216, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=238, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=260, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
    { .x=10, .y=282, .width=220, .height=22, .fontSize = SMALL, .xTextOffset = 7, .yTextOffset = 20 },
};

void TFTMenuPrepare()
{
        tft.fillRect(0, 0, 240, 320, TFT_COLOR_BGR);
        tft.drawRect(configForms[0].x - 1, configForms[0].y - 1, configForms[0].width + 2, configForms[0].height + 2, TFT_COLOR_FRM_LIN);
        tft.fillRect(configForms[0].x, configForms[0].y, configForms[0].width, configForms[0].height, TFT_COLOR_FRM_BGR);
        tft.drawRect(configForms[1].x - 1, configForms[1].y - 1, configForms[1].width + 2, configForms[1].height + 2, TFT_COLOR_FRM_LIN);
        tft.fillRect(configForms[1].x, configForms[1].y, configForms[1].width, configForms[1].height, TFT_COLOR_FRM_BGR);
}

void TFTMenuPrint(const byte Aim, String Content, const int Color)
{
        Content = Content.substring(0, 20);
        tft.fillRect(configFields[Aim].x, configFields[Aim].y, configFields[Aim].width, configFields[Aim].height, TFT_COLOR_FRM_BGR);
        tft.setFreeFont(configFields[Aim].fontSize ? &TFT_FONT_SMALL : &TFT_FONT_LARGE);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(configFields[Aim].x + configFields[Aim].xTextOffset, configFields[Aim].y + configFields[Aim].yTextOffset);
        tft.print(Content);
}

void TFTMenuPrintAddress(const byte Aim, String Content1, String Content2, String Content3, const int ColorNormal, const int ColorHighlight)
{
        tft.setFreeFont(configFields[Aim].fontSize ? &TFT_FONT_SMALL : &TFT_FONT_LARGE);
        tft.setCursor(configFields[Aim].x + configFields[Aim].xTextOffset, configFields[Aim].y + configFields[Aim].yTextOffset);
        tft.setTextColor(ColorNormal, TFT_COLOR_FRM_BGR);
        tft.print(Content1);
        tft.setTextColor(ColorHighlight, TFT_COLOR_FRM_BGR);
        tft.print(Content2);
        tft.setTextColor(ColorNormal, TFT_COLOR_FRM_BGR);
        tft.print(Content3);
}