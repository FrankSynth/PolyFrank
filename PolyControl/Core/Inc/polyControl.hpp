#pragma once

#include "debughelper/debughelper.hpp"
#include "fmc.h"
#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "livedata/liveData.hpp"
#include "ltdc.h"
#include "mdma.h"
#include "midiInterface/MIDIInterface.h"
#include "poly.hpp"
#include "preset/preset.hpp"
#include "spi.h"
#include "tim.h"
#include "usbd_midi_if.hpp"
#include <functional>

extern GUI ui;

extern midi::MidiInterface<COMusb> mididevice;

extern GlobalSettings globalSettings;

void PolyControlInit();
void PolyControlRun();
