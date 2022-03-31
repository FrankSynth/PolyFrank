#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include <functional>

typedef enum { DISABLED, NOTCONNECT, WAITFORRESPONSE, READY } interChipState;

namespace FlagHandler {

#ifdef POLYCONTROL

// InterChip send flags

extern bool layerActive[2];

extern bool HID_Initialized;

extern bool Control_Encoder_Interrupt;
extern std::function<void()> Control_Encoder_ISR;
extern elapsedMicros Control_Encoder_Interrupt_Timer;

extern bool Control_Touch_Interrupt;
extern std::function<void()> Control_Touch_ISR;

extern bool Panel_0_Touch_Interrupt;
extern std::function<void()> Panel_0_Touch_ISR;

extern bool Panel_1_Touch_Interrupt;
extern std::function<void()> Panel_1_Touch_ISR;

extern bool Panel_0_EOC_Interrupt;
extern bool Panel_1_EOC_Interrupt;
extern std::function<void()> Panel_EOC_ISR;

extern interChipState renderChip_State[2][2];
extern bool renderChipAwaitingData[2][2];
extern bool receiveDMARunning;

extern elapsedMillis renderChip_StateTimeout[2][2];

extern bool USB_HS_CONNECTED;
extern bool USB_FS_CONNECTED;

// Display
extern bool renderingDoneSwitchBuffer;

extern bool readTemperature;
extern std::function<void()> readTemperature_ISR;

#elif POLYRENDER

// InterChip receive flags

extern bool cvDacStarted[10];
extern bool cvDacFinished[10];
extern bool cvDacLastFinished[3];

extern bool renderNewCV;
extern bool cvRendered;
extern bool cvSent;
extern void (*renderNewCVFunc)();
extern void (*sendRenderedCVsFunc)();

#endif

extern bool interChipSend_DMA_Started;
extern bool interChipSend_DMA_Finished;
extern std::function<uint8_t()> interChipSend_DMA_FinishedFunc;

void handleFlags();
void initFlagHandler();

}; // namespace FlagHandler
