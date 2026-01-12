#pragma once
#include <Arduino.h>

enum TimerEventType {
    TIMER_EXPIRED,
    TIMER_TICK
};

struct TimerEvent {
    TimerEventType type;
    uint8_t        id;
};

void TimerTask(void* arg);

void timerStart(TimerID id, uint32_t ms, bool periodic);
void timerStop(TimerID id);
