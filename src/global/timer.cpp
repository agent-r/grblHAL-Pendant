
#include <Arduino.h>
#include "global/timer.h"
#include "global/global.h"

struct SoftTimer {
    bool active;
    bool periodic;
    uint32_t intervalMs;
    uint32_t remainingMs;
    uint8_t id;
};

#define TIMER_TICK_MS 10
#define MAX_TIMERS 10

static SoftTimer timers[MAX_TIMERS];

QueueHandle_t timerQueue = nullptr;

///////////////////////////// TIMER TASK /////////////////////////////////////////////////////////////////

void TimerTask(void* arg) {

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(TIMER_TICK_MS));

        for (int i = 0; i < MAX_TIMERS; i++) {

            if (!timers[i].active) { continue; }

            if (timers[i].remainingMs >= TIMER_TICK_MS) { timers[i].remainingMs -= TIMER_TICK_MS; }
            else { timers[i].remainingMs = 0; }

            if (timers[i].remainingMs == 0) 
            {

                TimerEvent ev = { .type = TIMER_EXPIRED, .id = timers[i].id };
                xQueueSend(timerQueue, &ev, 0);

                if (timers[i].periodic){ timers[i].remainingMs = timers[i].intervalMs; }
                else{ timers[i].active = false; }

            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////


void timerStart(TimerID id, uint32_t ms, bool periodic)
{
    for (int i = 0; i < MAX_TIMERS; i++)
    {
        if (timers[i].active && timers[i].id == id)
        {
            timers[i].intervalMs  = ms;
            timers[i].remainingMs = ms;
            timers[i].periodic    = periodic;
            return;
        }
        
        else if (!timers[i].active)
        {
            timers[i].active      = true;
            timers[i].periodic    = periodic;
            timers[i].intervalMs  = ms;
            timers[i].remainingMs = ms;
            timers[i].id          = id;
            return;
        }
    }
}


void timerStop(TimerID id) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].active && timers[i].id == id)
            timers[i].active = false;
    }
}
