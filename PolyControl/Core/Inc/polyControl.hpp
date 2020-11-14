#pragma once

#include "debughelper/debughelper.hpp"
#include "gfx/gui.hpp"
#include "livedata/liveData.hpp"
#include "mdma.h"
#include "midiInterface/MIDIInterface.h"
#include "poly.hpp"
#include "spi.h"
#include "tim.h"
#include "usbd_midi_if.hpp"
#include <functional>

extern GUI ui;

extern midi::MidiInterface<COMusb> mididevice;

void PolyControlInit();
void PolyControlRun();

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
