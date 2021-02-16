#pragma once

#include "debughelper/debughelper.hpp"
#include "fmc.h"
#include "gfx/gui.hpp"
#include "globalsettings/globalSettings.hpp"
#include "humanInterface/hid.hpp"
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

extern midi::MidiInterface<midiUSB::COMusb> mididevice;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

void PolyControlInit();
void PolyControlRun();
