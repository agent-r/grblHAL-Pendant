#include <Arduino.h>
#include <ArduinoJson.h>

#include "controller/controller.h"
#include "debug/debug.h"
#include "controller/configmenu.h"
#include "display/display_functions.h"
#include "global/global.h"
#include "global/timer.h"

// -------------------------------------------------------------------

void ControlTask(void *pv)
{

  static InputEvent inEv = {};

  systemState = STATE_RUN;

  static JsonDocument rxJsonDoc;

  static char gCode[BLE_TX_MAX_LEN];
  static char gCodeMessage[DISP_MAX_MENU_TEXT];

  static bool probeCompleted = false;
  static bool probe_triggered = false;

  resetSleepTimer();

  for (;;)
  {

    // -------- Input Events --------
    if (xQueueReceive(inputQueue, &inEv, 0))
    {

      switch (systemState)
      {

      case STATE_RUN:

        if (inEv.type == ENC_MOVE)
        {
          float distance = cnc.jogFactor * inEv.delta * cnc.axis[cnc.activeAxisIndex].direction;
          snprintf(gCode, sizeof(gCode), "$J=G91%c%.3fF%d", cnc.axis[cnc.activeAxisIndex].name, distance, cnc.axis[cnc.activeAxisIndex].jogSpeed);
          snprintf(gCodeMessage, sizeof(gCodeMessage), "Jog %c%.3f", cnc.axis[cnc.activeAxisIndex].name, distance);
          sendGCode("gcode", gCode, gCodeMessage);
        }
        if (inEv.type == BTN_AXIS_PREV)
        {
          if (cnc.activeAxisIndex <= 0) { cnc.activeAxisIndex = 3; }
          else { cnc.activeAxisIndex--; }

          DisplayEvent dispEvAxisDecrease = {};
          dispEvAxisDecrease.state.activeAxisIndex = cnc.activeAxisIndex;

          char str[8]; 
          dtostrf(cnc.axis[0].position,8,2,str); snprintf(dispEvAxisDecrease.state.x, DISP_MAX_TEXT, "%sX %s", (cnc.activeAxisIndex == 0) ? ">" : " " , str);
          dtostrf(cnc.axis[1].position,8,2,str); snprintf(dispEvAxisDecrease.state.y, DISP_MAX_TEXT, "%sY %s", (cnc.activeAxisIndex == 1) ? ">" : " " , str);
          dtostrf(cnc.axis[2].position,8,2,str); snprintf(dispEvAxisDecrease.state.z, DISP_MAX_TEXT, "%sZ %s", (cnc.activeAxisIndex == 2) ? ">" : " " , str);
          dtostrf(cnc.axis[3].position,8,2,str); snprintf(dispEvAxisDecrease.state.a, DISP_MAX_TEXT, "%sA %s", (cnc.activeAxisIndex == 3) ? ">" : " " , str);
          dispEvAxisDecrease.state.axisChanged = true;
          dispEvAxisDecrease.type = DISP_UPDATE_STATUS;
          xQueueSend(displayQueue, &dispEvAxisDecrease, 0);

        }
        if (inEv.type == BTN_AXIS_NEXT)
        {
          if (cnc.activeAxisIndex >= 3) { cnc.activeAxisIndex = 0; }
          else { cnc.activeAxisIndex++; }

          DisplayEvent dispEvAxisIncrease = {};

          dispEvAxisIncrease.state.activeAxisIndex = cnc.activeAxisIndex;

          char str[8]; 
          dtostrf(cnc.axis[0].position,8,2,str); snprintf(dispEvAxisIncrease.state.x, DISP_MAX_TEXT, "%sX %s", (cnc.activeAxisIndex == 0) ? ">" : " " , str);
          dtostrf(cnc.axis[1].position,8,2,str); snprintf(dispEvAxisIncrease.state.y, DISP_MAX_TEXT, "%sY %s", (cnc.activeAxisIndex == 1) ? ">" : " " , str);
          dtostrf(cnc.axis[2].position,8,2,str); snprintf(dispEvAxisIncrease.state.z, DISP_MAX_TEXT, "%sZ %s", (cnc.activeAxisIndex == 2) ? ">" : " " , str);
          dtostrf(cnc.axis[3].position,8,2,str); snprintf(dispEvAxisIncrease.state.a, DISP_MAX_TEXT, "%sA %s", (cnc.activeAxisIndex == 3) ? ">" : " " , str);

          dispEvAxisIncrease.state.axisChanged = true;
          dispEvAxisIncrease.type = DISP_UPDATE_STATUS;

          xQueueSend(displayQueue, &dispEvAxisIncrease, 0);

        }
        if (inEv.type == BTN_SET_ZERO)
        {
          snprintf(gCode, sizeof(gCode), "G10L20P0%c0", cnc.axis[cnc.activeAxisIndex].name);
          snprintf(gCodeMessage, sizeof(gCodeMessage), "Set %c 0", cnc.axis[cnc.activeAxisIndex].name);
          sendGCode("gcode", gCode, gCodeMessage);
        }

        if (inEv.type == BTN_GOTO_ZERO)
        {
          snprintf(gCode, sizeof(gCode), "$J=G90 %c0 F%d", cnc.axis[cnc.activeAxisIndex].name, cnc.axis[cnc.activeAxisIndex].jogSpeed);
          snprintf(gCodeMessage, sizeof(gCodeMessage), "GoTo %c 0",  cnc.axis[cnc.activeAxisIndex].name);
          sendGCode("gcode", gCode, gCodeMessage);
        }
        if (inEv.type == BTN_PROBE)
        {
          systemState = STATE_CONFIRM_PROBING;
          timerStart(TIMER_CONFIRM_PROBING, CONFIRM_TIMEOUT_MS, false);
          showMessage("Confirm: ENTER");
        }
        if (inEv.type == BTN_MENU)
        {
          systemState = STATE_CONFIGMENU;

          xQueueReset(bleRxQueue);
          xQueueReset(displayQueue);
          xQueueReset(inputQueue);

          initMenu();
        }
        if (inEv.type == BTN_FACTOR_DOWN)
        {

          DisplayEvent dispEvFactorDecrease = {};

          if (cnc.jogFactor <= 0.011) { cnc.jogFactor = 100; }
          else { cnc.jogFactor = cnc.jogFactor / 10; }
          char strfactor[6]; dtostrf(cnc.jogFactor,6,2,strfactor);
          snprintf(dispEvFactorDecrease.state.jogFactor, sizeof(dispEvFactorDecrease.state.jogFactor), " F   %s", strfactor);
          dispEvFactorDecrease.state.jogFactorChanged= true;
          dispEvFactorDecrease.type = DISP_UPDATE_STATUS;

          xQueueSend(displayQueue, &dispEvFactorDecrease, 0);

        }
        if (inEv.type == BTN_FACTOR_UP)
        {

          DisplayEvent dispEvFactorIncrease = {};

          if (cnc.jogFactor >= 99) { cnc.jogFactor = 0.01; }
          else { cnc.jogFactor = cnc.jogFactor * 10; }
          char strfactor[6]; dtostrf(cnc.jogFactor,6,2,strfactor);
          snprintf(dispEvFactorIncrease.state.jogFactor, sizeof(dispEvFactorIncrease.state.jogFactor), " F   %s", strfactor);
          dispEvFactorIncrease.state.jogFactorChanged = true;
          dispEvFactorIncrease.type = DISP_UPDATE_STATUS;

          xQueueSend(displayQueue, &dispEvFactorIncrease, 0);

        }
        if (inEv.type == BTN_HOME)
        {
            systemState = STATE_CONFIRM_HOMING;
            timerStart(TIMER_CONFIRM_HOMING, CONFIRM_TIMEOUT_MS, false);
            showMessage("Confirm: ENTER");
        }
        if (inEv.type == BTN_STOP)
        {
          sendGCode("cmd", "STOP", "Stop");
        }
        if (inEv.type == BTN_UNLOCK)
        {
          sendGCode("cmd", "UNLOCK", "Unlock");
       }
        if (inEv.type == BTN_ENTER)
        {
          sendGCode("cmd", "START", "Start");
        }
        break;

      case STATE_CONFIRM_HOMING:

        if (inEv.type == BTN_ENTER)
        {
          timerStop(TIMER_CONFIRM_HOMING);
          systemState = STATE_RUN;
          sendGCode("cmd", "HOME", "Home All");
        }
      break;

      case STATE_CONFIRM_PROBING:

        if (inEv.type == BTN_ENTER)
        {
          timerStop(TIMER_CONFIRM_PROBING);
          sendGCode("gcode", "G91", "Probing");
          snprintf(gCodeMessage, sizeof(gCodeMessage), "G38.2Z-%dF%d", config.ProbeDepth, config.ProbeSpeed);
          sendGCode("gcode", gCodeMessage, "Probing");;
          timerStart(TIMER_PROBING, config.ProbeTime * 1000, false);
          systemState = STATE_PROBING;
          probe_triggered = false;
          probeCompleted = false;

        }
      break;

      case STATE_CONFIGMENU:

        if (inEv.type == ENC_MOVE)
        {
          updateMenu(inEv);
        }
        if (inEv.type == BTN_ENTER)
        {
          updateMenu(inEv);
        }
        break;

        default:
        break;
      }

      resetSleepTimer();
    }

    // --------- BLE RX Event ------------
    BLERxEvent bleRxEv = {};

    if (xQueueReceive(bleRxQueue, &bleRxEv, 0))
    {
      if  (systemState == STATE_RUN || systemState == STATE_CONFIRM_HOMING || systemState == STATE_CONFIRM_PROBING || systemState == STATE_PROBING)
      {
        char bleRxJson[BLE_RX_MAX_LEN + 1];
        size_t copyLen = bleRxEv.len;

        if (copyLen >= BLE_RX_MAX_LEN)
        {
          copyLen = BLE_RX_MAX_LEN - 1;
        }

        memcpy(bleRxJson, bleRxEv.json, copyLen);
        bleRxJson[copyLen] = '\0';

        debugLog(DBG_INFO, "CTRL", bleRxJson);

        DeserializationError error = deserializeJson(rxJsonDoc, bleRxJson);

        if (error)
        {
          debugLog(DBG_ERROR, "CTRL", "JSON -> deserializeJson() failed");
        }

        DisplayEvent dispEvState = {};

        if (rxJsonDoc["state"].is<const char*>())
        {

          const char* state = rxJsonDoc["state"];
          snprintf(cnc.machineState, DISP_MAX_TEXT_MESSAGE_STATE, "%s", state);

          if (strcmp(cnc.machineState, cnc.machineStateOld) != 0) {

            /*
            if (systemState == STATE_PROBING && (strcmp(cnc.machineStateOld, "RUN") == 0) && (strcmp(cnc.machineState, "IDLE") == 0)) 
            {
                snprintf(gCodeMessage, sizeof(gCodeMessage), "G10L20P0Z%d", config.ProbeOffset);
                sendGCode("gcode", gCodeMessage, "Probing OK");;
                snprintf(gCodeMessage, sizeof(gCodeMessage), "$J=G91Z%dF%d", config.ProbeReturn, cnc.axis[2].jogSpeed);
                sendGCode("gcode", gCodeMessage, "Probing OK");;

                systemState == STATE_RUN;
            }
            */

            snprintf(dispEvState.state.machineState, DISP_MAX_TEXT_MESSAGE_STATE, "%s", cnc.machineState);
            strncpy(cnc.machineStateOld, cnc.machineState, DISP_MAX_TEXT_MESSAGE_STATE - 1);
            dispEvState.state.stateChanged = true;
          }
        }

        if (rxJsonDoc["wx"].is<float>())
        {
          cnc.axis[0].position = rxJsonDoc["wx"];
          char strwx[8]; dtostrf(cnc.axis[0].position,8,2,strwx);
          if (cnc.axis[0].position != cnc.axis[0].positionOld) {
            snprintf(dispEvState.state.x, DISP_MAX_TEXT, "%sX %s", (cnc.activeAxisIndex == 0) ? ">" : " " , strwx);
            cnc.axis[0].positionOld = cnc.axis[0].position;
            dispEvState.state.xChanged = true;
          }
      }

        if (rxJsonDoc["wy"].is<float>())
        {
          cnc.axis[1].position = rxJsonDoc["wy"];
          char strwy[8]; dtostrf(cnc.axis[1].position,8,2,strwy);
          if (cnc.axis[1].position != cnc.axis[1].positionOld) {
            snprintf(dispEvState.state.y, DISP_MAX_TEXT, "%sY %s", (cnc.activeAxisIndex == 1) ? ">" : " " , strwy);
            cnc.axis[1].positionOld = cnc.axis[1].position;
            dispEvState.state.yChanged = true;
          }
        }

        if (rxJsonDoc["wz"].is<float>())
        {
          cnc.axis[2].position = rxJsonDoc["wz"];
          char strwz[8]; dtostrf(cnc.axis[2].position,8,2,strwz);
          if (cnc.axis[2].position != cnc.axis[2].positionOld) {
            snprintf(dispEvState.state.z, DISP_MAX_TEXT, "%sZ %s", (cnc.activeAxisIndex == 2) ? ">" : " " , strwz);
            cnc.axis[2].positionOld = cnc.axis[2].position;
            dispEvState.state.zChanged = true;
          }
        }

        if (rxJsonDoc["wa"].is<float>())
        {
          cnc.axis[3].position = rxJsonDoc["wa"];
          char strwa[8]; dtostrf(cnc.axis[3].position,8,2,strwa);
          if (cnc.axis[3].position != cnc.axis[3].positionOld) {
            snprintf(dispEvState.state.a, DISP_MAX_TEXT, "%sA %s", (cnc.activeAxisIndex == 3) ? ">" : " " , strwa);
            cnc.axis[3].positionOld = cnc.axis[3].position;
            dispEvState.state.aChanged = true;
          }
        }

        if (rxJsonDoc["probe"].is<const char*>())
        {
          const char* probe = rxJsonDoc["probe"];

          if (strcmp(probe, "none") == 0) { }
          else if (strcmp(probe, "triggered") == 0)
          {
            probe_triggered = true;
          }
          else if (strcmp(probe, "finished") == 0) 
          {
            probeCompleted = true;
          }
        }

        dispEvState.type = DISP_UPDATE_STATUS;
        xQueueSend(displayQueue, &dispEvState, 0);

      }

    }


    // --------- BLE Scan ------------
    BLEScanEvent bleScanEv = {};
    if (xQueueReceive(bleScanEventQueue, &bleScanEv, 0))
    {
      handleBLEScanEvent(bleScanEv);
    }

    // --------- Timer Events ------------

    TimerEvent timerEv = {};
    if (xQueueReceive(timerQueue, &timerEv, 0) )
    {

      if (systemState == STATE_CONFIRM_HOMING)
      {
        if (timerEv.type == TIMER_EXPIRED && timerEv.id == TIMER_CONFIRM_HOMING)
        {
          systemState = STATE_RUN;
        }
      }

      if (systemState == STATE_CONFIRM_PROBING)
      {
        if (timerEv.type == TIMER_EXPIRED && timerEv.id == TIMER_CONFIRM_PROBING)
        {
          systemState = STATE_RUN;
        }
      }

      if (timerEv.type == TIMER_EXPIRED && timerEv.id == TIMER_MESSAGE_SHOW) 
      {
        DisplayEvent dispEvMessageClear = {};

        dispEvMessageClear.type = DISP_UPDATE_STATUS;
        snprintf(dispEvMessageClear.state.message, DISP_MAX_TEXT, "%s", " ");
        dispEvMessageClear.state.messageChanged = true;

        xQueueSend(displayQueue, &dispEvMessageClear, 0);

      }

      if (timerEv.type == TIMER_EXPIRED && timerEv.id == TIMER_SLEEP)
      {
            BLECmd cmd = { BLE_DISCONNECT };
            xQueueSend(bleCmdQueue, &cmd, 0);

            DisplayEvent dispEvSleep = {};
            dispEvSleep.type = DISP_SLEEP;
            xQueueSend(displayQueue, &dispEvSleep, 0);

            vTaskDelay(pdMS_TO_TICKS(200));

            debugLog(DBG_INFO, "CTRL", "Entering deep sleep");

            esp_sleep_enable_ext0_wakeup( static_cast<gpio_num_t>(ENCODER_PIN_A), 1 );
            // esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW); Wakeup - several pins?

            esp_deep_sleep_start();
      }


      if (timerEv.type == TIMER_EXPIRED && timerEv.id == TIMER_PROBING) 
      {
        
        DisplayEvent dispEvMessageClear = {};

        dispEvMessageClear.type = DISP_UPDATE_STATUS;
        snprintf(dispEvMessageClear.state.message, DISP_MAX_TEXT_MESSAGE_STATE, "Probing Aborted", " ");
        dispEvMessageClear.state.messageChanged = true;

        xQueueSend(displayQueue, &dispEvMessageClear, 0);

        systemState = STATE_RUN;

      }
    }


    if (systemState == STATE_PROBING) 
    {
      if (probe_triggered)
      {
        timerStop(TIMER_PROBING);

        debugLog(DBG_INFO, "PROBE", "TRIGGER RECEIVED");

        sendGCode("gcode", "G90", "Probing");
        snprintf(gCodeMessage, sizeof(gCodeMessage), "G10L20P0Z%d", config.ProbeOffset);
        sendGCode("gcode", gCodeMessage, "Probing");;
        snprintf(gCodeMessage, sizeof(gCodeMessage), "$J=G91Z%dF%d", config.ProbeReturn, cnc.axis[2].jogSpeed);
        sendGCode("gcode", gCodeMessage, "Probing");;

        systemState = STATE_RUN;

        probe_triggered = false;
        probeCompleted = false;
      }
      else if (probeCompleted)
      {
        timerStop(TIMER_PROBING);

        debugLog(DBG_INFO, "PROBE", "COMPLETED RECEIVED");

        sendGCode("gcode", "G90", "Probing Error");
        snprintf(gCodeMessage, sizeof(gCodeMessage), "$J=G91Z%dF%d", config.ProbeReturn, cnc.axis[2].jogSpeed);
        sendGCode("gcode", gCodeMessage, "Probing");;
        
        systemState = STATE_RUN;

        probe_triggered = false;
        probeCompleted = false;
      }
    }

    vTaskDelay(1);
  }
}


////////////////////////////////////////////

void sendGCode(const char* type, const char* gcode, const char* message) {

  BLETxEvent ev = {};

  JsonDocument txJsonDoc;
  txJsonDoc[type] = gcode;
  ev.len = serializeJson(txJsonDoc, ev.json, sizeof(ev.json));

  xQueueSend(bleTxQueue, &ev, 0);

  showMessage(message);

}


void showMessage(const char* message) {

  DisplayEvent dispEvMessage = {};

  dispEvMessage.type = DISP_UPDATE_STATUS;
  snprintf(dispEvMessage.state.message, DISP_MAX_TEXT_MESSAGE_STATE, "%s", message);
  dispEvMessage.state.messageChanged = true;

  xQueueSend(displayQueue, &dispEvMessage, 0);

  timerStart(TIMER_MESSAGE_SHOW, MESSAGE_TIMEOUT_MS, false);
  
}


void resetSleepTimer()
{

    if (config.SleepTime > 0) {
      timerStart(TIMER_SLEEP, config.SleepTime * 60000, false);
    }
    else {
      timerStop(TIMER_SLEEP);
    }

}