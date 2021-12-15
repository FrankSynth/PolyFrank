#include "flagHandler.hpp"
#include "debughelper/debughelper.hpp"

namespace FlagHandler {

void initFlagHandler() {
#ifdef POLYCONTROL

    renderingDoneSwitchBuffer = false;

    for (uint8_t i = 0; i < 2; i++) {

        interChipA_MDMA_Started[i] = false;
        interChipA_MDMA_Finished[i] = false;
        interChipA_MDMA_FinishedFunc[i] = nullptr;
        interChipA_DMA_Started[i] = false;
        interChipA_DMA_Finished[i] = false;
        interChipA_DMA_FinishedFunc[i] = nullptr;
        // interChipB_MDMA_Started[i] = false;
        // interChipB_MDMA_Finished[i] = false;
        // interChipB_MDMA_FinishedFunc[i] = nullptr;
        // interChipB_DMA_Started[i] = false;
        // interChipB_DMA_Finished[i] = false;
        // interChipB_DMA_FinishedFunc[i] = nullptr;
    }

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

    interChipReceive_DMA_Started = false;
    interChipReceive_DMA_Finished = false;
    interChipReceive_MDMA_Started = false;
    interChipReceive_MDMA_Finished = false;
    interChipReceive_DMA_FinishedFunc = nullptr;
    interChipReceive_MDMA_FinishedFunc = nullptr;

    // saiHalfCptl = false;
    // saiHalfCptlFunc = nullptr;
    // saiCptl = false;
    // saiCptlFunc = nullptr;

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
}

#ifdef POLYCONTROL

// InterChip send flags
bool interChipA_MDMA_Started[2];
bool interChipA_MDMA_Finished[2];
std::function<uint8_t()> interChipA_MDMA_FinishedFunc[2];
bool interChipA_DMA_Started[2];
bool interChipA_DMA_Finished[2];
std::function<uint8_t()> interChipA_DMA_FinishedFunc[2];
// bool interChipB_MDMA_Started[2];
// bool interChipB_MDMA_Finished[2];
// std::function<uint8_t()> interChipB_MDMA_FinishedFunc[2];
// bool interChipB_DMA_Started[2];
// bool interChipB_DMA_Finished[2];
// std::function<uint8_t()> interChipB_DMA_FinishedFunc[2];

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
interChipState interChipA_State[2] = {NOTCONNECT, NOTCONNECT}; // init State wait for ready
interChipState interChipB_State[2] = {NOTCONNECT, NOTCONNECT}; // init State wait for ready

elapsedMillis interChipA_StateTimeout[2] = {0, 0};
elapsedMillis interChipB_StateTimeout[2] = {0, 0};

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
bool interChipReceive_MDMA_Started;
bool interChipReceive_MDMA_Finished;
std::function<uint8_t()> interChipReceive_MDMA_FinishedFunc;
bool interChipReceive_newDataAvailable;
std::function<uint8_t()> interChipReceive_newDataAvailableFunc;

// bool saiHalfCptl;
// std::function<uint8_t()> saiHalfCptlFunc;
// bool saiCptl;
// std::function<uint8_t()> saiCptlFunc;

bool cvDacStarted[10];
bool cvDacFinished[10];
bool cvDacLastFinished[3];

// std::function<void()> cvDacCFinishedFunc;

bool renderNewCV;
bool cvRendered;
bool cvSent;
std::function<void()> renderNewCVFunc;
std::function<void()> sendRenderedCVsFunc;
#endif

// handle all interrupts
void handleFlags() {

#ifdef POLYCONTROL

    for (uint8_t i = 0; i < 2; i++) {

        // InterChip send flags
        if (interChipA_MDMA_Finished[i]) {
            if (interChipA_MDMA_FinishedFunc[i] != nullptr) {
                if (interChipA_MDMA_FinishedFunc[i]() == 0) {
                    interChipA_MDMA_FinishedFunc[i] = nullptr;
                    interChipA_MDMA_Finished[i] = 0;
                }
            }
            else {
                interChipA_MDMA_Finished[i] = 0;
            }
        }
        if (interChipA_DMA_Finished[i]) {
            if (interChipA_DMA_FinishedFunc[i] != nullptr) {
                if (interChipA_DMA_FinishedFunc[i]() == 0) {
                    interChipA_DMA_FinishedFunc[i] = nullptr;
                    interChipA_DMA_Finished[i] = 0;
                }
            }
            else {
                interChipA_DMA_Finished[i] = 0;
            }
        }
        // if (interChipB_MDMA_Finished[i]) {
        //     if (interChipB_MDMA_FinishedFunc[i] != nullptr) {
        //         if (interChipB_MDMA_FinishedFunc[i]() == 0) {
        //             interChipB_MDMA_FinishedFunc[i] = nullptr;
        //             interChipB_MDMA_Finished[i] = 0;
        //         }
        //     }
        //     else {
        //         interChipB_MDMA_Finished[i] = 0;
        //     }
        // }
        // if (interChipB_DMA_Finished[i]) {
        //     if (interChipB_DMA_FinishedFunc[i] != nullptr) {
        //         if (interChipB_DMA_FinishedFunc[i]() == 0) {
        //             interChipB_DMA_FinishedFunc[i] = nullptr;
        //             interChipB_DMA_Finished[i] = 0;
        //         }
        //     }
        //     else {
        //         interChipB_DMA_Finished[i] = 0;
        //     }
        // }
    }
    for (uint8_t i = 0; i < 2; i++) {
        if (interChipA_State[i] != READY) {
            if (interChipA_StateTimeout[i] > 10) { // 10ms timeout
                if (interChipA_State[i] == WAITFORRESPONSE) {
                    //  PolyError_Handler("ERROR | FATAL | Communication -> layerChip A -> no reponse");
                }
                else if (interChipA_State[i] == DATARECEIVED) {
                    // PolyError_Handler("ERROR | FATAL | Communication -> layerChip A -> Checksum failed");
                }
            }
        }
        if (interChipB_State[i] != READY) {
            if (interChipB_StateTimeout[i] > 10) { // 10ms timeout
                if (interChipB_State[i] == WAITFORRESPONSE) {
                    // PolyError_Handler("ERROR | FATAL | Communication -> layerChip B -> no reponse");
                }
                else if (interChipB_State[i] == DATARECEIVED) {
                    // PolyError_Handler("ERROR | FATAL | Communication -> layerChip B -> Checksum failed");
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

    // InterChip receive flags
    if (interChipReceive_DMA_Finished) {
        if (interChipReceive_DMA_FinishedFunc != nullptr) {
            if (interChipReceive_DMA_FinishedFunc() == 0) {
                interChipReceive_DMA_FinishedFunc = nullptr;
                interChipReceive_DMA_Finished = 0;
            }
        }
        else {
            interChipReceive_DMA_Finished = 0;
        }
    }
    if (interChipReceive_MDMA_Finished) {
        if (interChipReceive_MDMA_FinishedFunc != nullptr) {
            if (interChipReceive_MDMA_FinishedFunc() == 0) {
                interChipReceive_MDMA_FinishedFunc = nullptr;
                interChipReceive_MDMA_Finished = 0;
            }
        }
        else {
            interChipReceive_MDMA_Finished = 0;
        }
    }

    // if (saiHalfCptl) {
    //     if (saiHalfCptlFunc != nullptr) {
    //         if (saiHalfCptlFunc() == 0) {
    //             saiHalfCptlFunc = nullptr;
    //             saiHalfCptl = 0;
    //         }
    //     }
    //     else {
    //         saiHalfCptl = 0;
    //     }
    // }

    // if (saiCptl) {
    //     if (saiCptlFunc != nullptr) {
    //         if (saiCptlFunc() == 0) {
    //             saiCptlFunc = nullptr;
    //             saiCptl = 0;
    //         }
    //     }
    //     else {
    //         saiCptl = 0;
    //     }
    // }

#endif
}

}; // namespace FlagHandler
