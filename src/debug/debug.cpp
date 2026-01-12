
#include <Arduino.h>
#include "global/global.h"
#include "debug/debug.h"

QueueHandle_t debugQueue = nullptr;

// ---------------------------------------------------------------------------------

void DebugTask(void *pv)
{
  Serial.begin(115200);

  DebugEvent ev = {};
  for (;;)
  {
    if (xQueueReceive(debugQueue, &ev, portMAX_DELAY))
    {
      Serial.print("[");
      Serial.print(ev.source);
      Serial.print("] ");
      Serial.println(ev.msg);
    }
  }
}

void debugLog(DebugLevel lvl, const char *src, const char *msg)
{
  if (!debugQueue)
    return;

  DebugEvent ev = {};
  ev.level = lvl;
  ev.source = src;
  strncpy(ev.msg, msg, DEBUG_MSG_LEN - 1);
  ev.msg[DEBUG_MSG_LEN - 1] = '\0';

  xQueueSend(debugQueue, &ev, 0);
}