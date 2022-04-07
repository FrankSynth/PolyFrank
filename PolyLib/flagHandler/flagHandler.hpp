#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include <functional>

typedef enum { DISABLED, NOTCONNECT, WAITFORRESPONSE, READY, INTERCHIPERROR } interChipState;

namespace FlagHandler {

#ifdef POLYCONTROL

// InterChip send flags

extern volatile bool layerActive[2];

extern volatile bool HID_Initialized;

extern volatile bool Control_Encoder_Interrupt;
extern std::function<void()> Control_Encoder_ISR;
extern elapsedMicros Control_Encoder_Interrupt_Timer;

extern volatile bool Control_Touch_Interrupt;
extern std::function<void()> Control_Touch_ISR;

extern volatile bool Panel_0_Touch_Interrupt;
extern std::function<void()> Panel_0_Touch_ISR;

extern volatile bool Panel_1_Touch_Interrupt;
extern std::function<void()> Panel_1_Touch_ISR;

extern volatile bool Panel_0_EOC_Interrupt;
extern volatile bool Panel_1_EOC_Interrupt;
extern std::function<void()> Panel_EOC_ISR;

extern volatile interChipState renderChip_State[2][2];
extern volatile bool renderChipAwaitingData[2][2];
// extern bool receiveDMARunning;

extern elapsedMillis renderChip_StateTimeout[2][2];

extern volatile bool USB_HS_CONNECTED;
extern volatile bool USB_FS_CONNECTED;

// Display
extern volatile bool renderingDoneSwitchBuffer;

extern volatile bool readTemperature;
extern std::function<void()> readTemperature_ISR;

#elif POLYRENDER

// InterChip receive flags

extern bool cvDacStarted[10];
extern bool cvDacFinished[10];
extern bool cvDacLastFinished[3];

extern bool renderNewCV;
extern void (*renderNewCVFunc)();

#endif

void handleFlags();
void initFlagHandler();

}; // namespace FlagHandler
