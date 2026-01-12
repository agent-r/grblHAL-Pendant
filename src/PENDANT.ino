
#include "bluetooth/bluetooth.h"
#include "controller/controller.h"
#include "debug/debug.h"
#include "display/display.h"
#include "global/global.h"
#include "global/eeprom.h"
#include "global/timer.h"
#include "input/input.h"

//////////////////////////////////////////////
//
//    TODOs:
//    -
//    - InfoScreen (vorher clearen!)
//    - Battery?
//    - Sleep - wakeup from several pins?
//    - EEPROM debug-write crashes!
//    - (re-)connect after Config
//
///////////////////////////////////////////////
//
//    - TEST BRIDGE:        GRBLHAL > 7C:9E:BD:62:46:02
//    - PRODUCTIVE BRIDGE:  GRBLHAL > 0C:B8:15:C3:B1:EA
//
//
/////////////////////////////////////////////

void setup()
{

  debugQueue = xQueueCreate(20, sizeof(DebugEvent));
  xTaskCreatePinnedToCore(DebugTask, "Debug", TASK_STACK_SIZE, NULL, DEBUG_TASK_PRIO, NULL, 0); // before loading eeprom

  // TaskHandle_t timerTaskHandle = nullptr;
  timerQueue = xQueueCreate(10, sizeof(TimerEvent));
  xTaskCreatePinnedToCore(TimerTask, "Timer", TASK_STACK_SIZE_TIMER, NULL, TIMER_TASK_PRIO, NULL, 1);

  // defaultsLoad();
  eepromLoad();

  inputQueue = xQueueCreate(10, sizeof(InputEvent));
  bleTxQueue = xQueueCreate(8, sizeof(BLETxEvent));
  bleRxQueue = xQueueCreate(8, sizeof(BLERxEvent));
  bleCmdQueue = xQueueCreate(8, sizeof(BLECmd));
  bleScanEventQueue = xQueueCreate(8, sizeof(BLEScanEvent));
  displayQueue = xQueueCreate(10, sizeof(DisplayEvent));

  xTaskCreatePinnedToCore(InputTask, "Input", TASK_STACK_SIZE, NULL, INPUT_TASK_PRIO, NULL, 0);
  xTaskCreatePinnedToCore(BLETask, "BLE", TASK_STACK_SIZE, NULL, BLE_TASK_PRIO, NULL, 0);

  xTaskCreatePinnedToCore(ControlTask, "Control", TASK_STACK_SIZE, NULL, CONTROL_TASK_PRIO, NULL, 1);
  xTaskCreatePinnedToCore(DisplayTask, "Display", TASK_STACK_SIZE, NULL, DISPLAY_TASK_PRIO, NULL, 1);
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}
