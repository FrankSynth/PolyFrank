#include "flagHandler.hpp"

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
        interChipB_MDMA_Started[i] = false;
        interChipB_MDMA_Finished[i] = false;
        interChipB_MDMA_FinishedFunc[i] = nullptr;
        interChipB_DMA_Started[i] = false;
        interChipB_DMA_Finished[i] = false;
        interChipB_DMA_FinishedFunc[i] = nullptr;
    }

    Control_Encoder_Interrupt = false;
    Control_Touch_Interrupt = false;
    Panel_0_Touch_Interrupt = false;
    Panel_1_Touch_Interrupt = false;

#elif POLYRENDER

    interChipReceive_DMA_Started = false;
    interChipReceive_DMA_Finished = false;
    interChipReceive_MDMA_Started = false;
    interChipReceive_MDMA_Finished = false;
    interChipReceive_DMA_FinishedFunc = nullptr;
    interChipReceive_MDMA_FinishedFunc = nullptr;

    saiHalfCptl = false;
    saiHalfCptlFunc = nullptr;
    saiCptl = false;
    saiCptlFunc = nullptr;
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
bool interChipB_MDMA_Started[2];
bool interChipB_MDMA_Finished[2];
std::function<uint8_t()> interChipB_MDMA_FinishedFunc[2];
bool interChipB_DMA_Started[2];
bool interChipB_DMA_Finished[2];
std::function<uint8_t()> interChipB_DMA_FinishedFunc[2];

bool Control_Encoder_Interrupt;
std::function<void()> Control_Encoder_ISR;

bool Control_Touch_Interrupt;
std::function<void()> Control_Touch_ISR;

bool Panel_0_Touch_Interrupt;
std::function<void()> Panel_0_Touch_ISR;

bool Panel_1_Touch_Interrupt;
std::function<void()> Panel_1_Touch_ISR;

// Display
bool renderingDoneSwitchBuffer;

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

bool saiHalfCptl;
std::function<uint8_t()> saiHalfCptlFunc;
bool saiCptl;
std::function<uint8_t()> saiCptlFunc;
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
        if (interChipB_MDMA_Finished[i]) {
            if (interChipB_MDMA_FinishedFunc[i] != nullptr) {
                if (interChipB_MDMA_FinishedFunc[i]() == 0) {
                    interChipB_MDMA_FinishedFunc[i] = nullptr;
                    interChipB_MDMA_Finished[i] = 0;
                }
            }
            else {
                interChipB_MDMA_Finished[i] = 0;
            }
        }
        if (interChipB_DMA_Finished[i]) {
            if (interChipB_DMA_FinishedFunc[i] != nullptr) {
                if (interChipB_DMA_FinishedFunc[i]() == 0) {
                    interChipB_DMA_FinishedFunc[i] = nullptr;
                    interChipB_DMA_Finished[i] = 0;
                }
            }
            else {
                interChipB_DMA_Finished[i] = 0;
            }
        }

        if (Control_Encoder_Interrupt) {
            if (Control_Encoder_ISR != nullptr) {
                Control_Encoder_ISR();
            }
            Control_Encoder_Interrupt = 0;
        }
        if (Control_Touch_Interrupt) {
            if (Control_Touch_ISR != nullptr) {
                Control_Touch_ISR();
                Panel_0_Touch_ISR(); // TODO rausnehmen nur temporär
            }
            Control_Touch_Interrupt = 0;
        }

        if (Panel_0_Touch_Interrupt) {
            if (Panel_0_Touch_ISR != nullptr) {
                Panel_0_Touch_ISR();
            }
            Panel_0_Touch_Interrupt = 0;
        }
        if (Panel_1_Touch_Interrupt) {
            if (Panel_1_Touch_ISR != nullptr) {
                Panel_1_Touch_ISR();
            }
            Panel_1_Touch_Interrupt = 0;
        }
    }

#elif POLYRENDER

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

    if (saiHalfCptl) {
        if (saiHalfCptlFunc != nullptr) {
            if (saiHalfCptlFunc() == 0) {
                saiHalfCptlFunc = nullptr;
                saiHalfCptl = 0;
            }
        }
        else {
            saiHalfCptl = 0;
        }
    }

    if (saiCptl) {
        if (saiCptlFunc != nullptr) {
            if (saiCptlFunc() == 0) {
                saiCptlFunc = nullptr;
                saiCptl = 0;
            }
        }
        else {
            saiCptl = 0;
        }
    }

#endif
}

}; // namespace FlagHandler
