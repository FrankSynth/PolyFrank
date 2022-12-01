#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include <functional>

typedef enum { DISABLED, NOTCONNECT, WAITFORRESPONSE, READY, INTERCHIPERROR } interChipState;

typedef enum {
    DRIVER_IDLE,
    DRIVER_START,
    DRIVER_0_TRANSMIT,
    DRIVER_1_TRANSMIT,

} ledDriverState;

namespace FlagHandler {

#ifdef POLYCONTROL

// InterChip send flags

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
extern std::function<void()> Panel_0_EOC_ISR;
extern std::function<void()> Panel_1_EOC_ISR;

extern std::function<void()> Panel_nextChannel_ISR;

extern volatile bool Panel_0_RXTX_Interrupt;
extern volatile bool Panel_1_RXTX_Interrupt;
extern std::function<void()> Panel_0_RXTX_ISR;
extern std::function<void()> Panel_1_RXTX_ISR;

extern volatile bool USB_HS_CONNECTED;
extern volatile bool USB_FS_CONNECTED;

extern volatile bool MIDI_USB_TRAFFIC;
extern volatile bool MIDI_DIN_TRAFFIC;
extern volatile bool COM_USB_TRAFFIC;
extern volatile bool COM_PRINT_ENABLE;

// Display
extern volatile bool renderingDoneSwitchBuffer;

extern volatile bool readTemperature;
extern std::function<void()> readTemperature_ISR;

extern volatile ledDriverState ledDriverATransmit;
extern volatile ledDriverState ledDriverBTransmit;
extern volatile ledDriverState ledDriverControlTransmit;

extern volatile bool ledDriverA_Interrupt;
extern volatile bool ledDriverB_Interrupt;
extern volatile bool ledDriverControl_Interrupt;

extern std::function<void()> ledDriverA_ISR[2];
extern std::function<void()> ledDriverB_ISR[2];
extern std::function<void()> ledDriverControl_ISR;

extern std::function<void()> ledDriverUpdateCurrent_ISR;

extern volatile bool ledDriverUpdateCurrent;

#elif POLYRENDER

extern volatile bool outputCollect;
extern volatile bool outputReady;
extern void (*outputCollectFunc)();

extern volatile bool renderNewCV;

#endif

void handleFlags();
void initFlagHandler();

}; // namespace FlagHandler
