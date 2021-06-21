
#pragma once

#include "globalsettings/globalSettings.hpp"
#include <algorithm>
#include <list>
#include <string>
#include <vector>

#define NUMBERVOICES 4
#define NUMBERCLOCKSTATES 23
#define NOTERANGE 88 // Supported Midi Note Range

#define MAXCLOCKTICKS 2304

typedef enum { LAYERA, LAYERB, LAYERAB } playModeEnum;
typedef enum { FREE, PLAY, SELECT, SUSTAIN } voiceStatusEnum;

typedef struct {
    uint8_t note;
    uint8_t velocity;
    uint8_t layerID;
    uint8_t released = 0;
    uint32_t born = 0; // in micros?
    uint32_t lifespan = 0;
} Key;

// struct pro voice für den aktuellen status: gespielter ton, c
typedef struct {
    voiceStatusEnum status = FREE;
    uint8_t note = 0;
    uint8_t velocity = 0;
    uint8_t voiceID = 0;
    uint8_t layerID = 0;
    uint32_t playID = 0;
} voiceStateStruct;

bool compareByNote(const Key &a, const Key &b);

extern uint16_t clockTicksPerStep[23];
