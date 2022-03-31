#include "flagHandler.hpp"
#include "debughelper/debughelper.hpp"

namespace FlagHandler {

void initFlagHandler() {
#ifdef POLYCONTROL

    renderingDoneSwitchBuffer = false;

    HID_Initialized = false;
    Panel_0_EOC_Interrupt = false;
    Panel_1_EOC_Interrupt = false;
    Panel_EOC_ISR = nullptr;

    Control_Touch_Interrupt = false;
    Control_Touch_ISR = nullptr;

    Panel_0_Touch_Interrupt = false;
    Panel_0_Touch_ISR = nullptr;

    Panel_1_Touch_Interrupt = false;
    Panel_1_Touch_ISR = nullptr;

    readTemperature = false;

#elif POLYRENDER

    for (uint32_t i = 0; i < 10; i++)
        cvDacStarted[i] = false;

    for (uint32_t i = 0; i < 10; i++)
        cvDacFinished[i] = false;

    for (uint32_t i = 0; i < 3; i++)
        cvDacLastFinished[i] = true;

    renderNewCV = false;
    cvRendered = false;
    cvSent = false;
    renderNewCVFunc = nullptr;
    sendRenderedCVsFunc = nullptr;

#endif

    interChipSend_DMA_Started = false;
    interChipSend_DMA_Finished = false;
    interChipSend_DMA_FinishedFunc = nullptr;
}

#ifdef POLYCONTROL

// InterChip send flags

bool layerActive[2] = {false};

bool receiveDMARunning = false;

bool Control_Encoder_Interrupt = false;
std::function<void()> Control_Encoder_ISR = nullptr;
elapsedMicros Control_Encoder_Interrupt_Timer;
bool HID_Initialized = false;

bool Panel_0_EOC_Interrupt = false;
bool Panel_1_EOC_Interrupt = false;
std::function<void()> Panel_EOC_ISR = nullptr;

bool Control_Touch_Interrupt = false;
std::function<void()> Control_Touch_ISR = nullptr;

bool Panel_0_Touch_Interrupt = false;
std::function<void()> Panel_0_Touch_ISR = nullptr;

bool Panel_1_Touch_Interrupt = false;
std::function<void()> Panel_1_Touch_ISR = nullptr;

/* interChipx_state:
    state == 0 -> Transmitted
    state == 1 -> Received
    state == 2 -> DMA copied
    state == 3 -> Checksum
    state == 4 -> READY
*/

interChipState renderChip_State[2][2] = {{NOTCONNECT, NOTCONNECT},
                                         {NOTCONNECT, NOTCONNECT}}; // init State wait for ready

bool renderChipAwaitingData[2][2] = {false};

elapsedMillis renderChip_StateTimeout[2][2] = {{0, 0}, {0, 0}};

bool USB_HS_CONNECTED = false;
bool USB_FS_CONNECTED = false;

// Display
bool renderingDoneSwitchBuffer;

bool readTemperature;
std::function<void()> readTemperature_ISR = nullptr;

#elif POLYRENDER

// InterChip receive flags
bool interChipReceive_DMA_Started;
bool interChipReceive_DMA_Finished;
std::function<uint8_t()> interChipReceive_DMA_FinishedFunc;
bool interChipReceive_newDataAvailable;
std::function<uint8_t()> interChipReceive_newDataAvailableFunc;

bool cvDacStarted[10];
bool cvDacFinished[10];
bool cvDacLastFinished[3];

bool renderNewCV;
bool cvRendered;
bool cvSent;
void (*renderNewCVFunc)();
void (*sendRenderedCVsFunc)();
#endif

bool interChipSend_DMA_Started;
bool interChipSend_DMA_Finished;
std::function<uint8_t()> interChipSend_DMA_FinishedFunc;

// handle all interrupts
void handleFlags() {

#ifdef POLYCONTROL

    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            if (renderChip_State[i][j] != READY) {
                if (renderChip_StateTimeout[i][j] > 100) { // 100ms timeout
                    if (renderChip_State[i][j] == WAITFORRESPONSE) {
                        if (renderChipAwaitingData[i][j]) {
                            PolyError_Handler("ERROR | FATAL | Communication > awaiting data, no reponse");
                        }
                        else {
                            PolyError_Handler("ERROR | FATAL | Communication > no ready");
                        }
                    }
                }
            }
        }
    }

    // HID Flags

    // HID ICs initialized
    if (HID_Initialized == true) {
        // Control Panel Encoder
        if (Control_Encoder_Interrupt) {
            if (Control_Encoder_Interrupt_Timer > 250) {
                if (Control_Encoder_ISR != nullptr) {
                    Control_Encoder_ISR();
                }

                // interrupt cleared?
                if (HAL_GPIO_ReadPin(Encoder_Interrupt_GPIO_Port, Encoder_Interrupt_Pin) == 1) {
                    Control_Encoder_Interrupt = false;
                }
            }
        }
        // Control Panel Touch
        if (Control_Touch_Interrupt) {
            // Control_Touch_Interrupt = 0;
            if (Control_Touch_ISR != nullptr) {
                Control_Touch_ISR();
            }
        }

        // Panel Touch
        if (Panel_0_Touch_Interrupt) {
            // Panel_0_Touch_Interrupt = 0;
            if (Panel_0_Touch_ISR != nullptr) {
                Panel_0_Touch_ISR();
            }
        }
        if (Panel_1_Touch_Interrupt) {
            // Panel_1_Touch_Interrupt = 0;
            if (Panel_1_Touch_ISR != nullptr) {
                Panel_1_Touch_ISR();
            }
        }

        // Panel ADC
        if (Panel_0_EOC_Interrupt && Panel_1_EOC_Interrupt) {
            Panel_0_EOC_Interrupt = 0;
            Panel_1_EOC_Interrupt = 0;

            if (Panel_EOC_ISR != nullptr) {
                Panel_EOC_ISR();
            }
        }
    }
    if (readTemperature) {
        readTemperature = false;
        if (readTemperature_ISR != nullptr) {
            readTemperature_ISR();
        }
    }

#elif POLYRENDER

    if (renderNewCV) {
        renderNewCV = false;
        renderNewCVFunc();
    }

    if (cvRendered && cvSent) {
        cvRendered = false;
        cvSent = false;
        sendRenderedCVsFunc();
    }

#endif

    // if (interChipSend_DMA_Finished) {
    //     if (interChipSend_DMA_FinishedFunc() == 0) {
    //         interChipSend_DMA_Finished = 0;
    //     }
    // }
}

}; // namespace FlagHandler
