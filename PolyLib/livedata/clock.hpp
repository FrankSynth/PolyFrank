#pragma once

#include "liveDataBase.hpp"


class Clock {
  public:
    void tick() {
        static elapsedMillis averagingStartTimer;
        static elapsedMicros tickTimer;
        static uint32_t accumMicrosPerTick = 0;
        static uint32_t averageCounter = 0;
        static uint8_t doNotCalcBpm = 0;

        counter++;
        counter %= MAXCLOCKTICKS;
        ticked = 1;

        averageCounter++;
        accumMicrosPerTick += tickTimer;
        tickTimer = 0;

        if (averagingStartTimer > 1000) {

            // avoid updating bpm when last update is too long ago
            if (averagingStartTimer > 2000 || doNotCalcBpm) {
                averagingStartTimer = 0;
                accumMicrosPerTick = 0;
                averageCounter = 0;
                doNotCalcBpm = 0;
                return;
            }
            bpm = 60000000. / ((((float)accumMicrosPerTick * 24) / (float)(averageCounter)));
            accumMicrosPerTick = 0;
            averageCounter = 0;
            averagingStartTimer = 0;
        }
    }

    uint8_t ticked = 0;

    uint32_t counter = 0;

    float bpm = 120;
};

class ClockSource {
  public:
    ClockSource() { clockBPM.useAcceleration = 1; } // enable acceleration on the BPM value
    void serviceRoutine() {
        static int32_t lastBPM = 0;
        if (lastBPM != clockBPM.value) {

            uint32_t periode = (60000000 / (clockBPM.value * 24)) - 1;
            __HAL_TIM_SET_AUTORELOAD(&htim5, periode);
            // htim5.Instance->ARR = periode;
            // println("set clockPeriod  : ", periode);
        }
        lastBPM = clockBPM.value;
    }
    Setting clockBPM = Setting("BPM", 120, 20, 350, false, binary);
};
