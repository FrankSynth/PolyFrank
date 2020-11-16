#include "flagHandler.hpp"

namespace FlagHandler {

void initFlagHandler() {

    interChipReceive_DMA_Started = false;
    interChipReceive_DMA_Finished = false;
    interChipReceive_MDMA_Started = false;
    interChipReceive_MDMA_Finished = false;
    interChipReceive_DMA_FinishedFunc = nullptr;
    interChipReceive_MDMA_FinishedFunc = nullptr;
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
}

// InterChip receive flags
bool interChipReceive_DMA_Started;
bool interChipReceive_DMA_Finished;
std::function<uint8_t()> interChipReceive_DMA_FinishedFunc;
bool interChipReceive_MDMA_Started;
bool interChipReceive_MDMA_Finished;
std::function<uint8_t()> interChipReceive_MDMA_FinishedFunc;

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

// Display
bool renderingDoneSwitchBuffer;

// handle all interrupts
void handleFlags() {

    for (uint8_t i = 0; i < 2; i++) {

        // InterChip send flags
        if (interChipA_MDMA_Finished) {
            if (interChipA_MDMA_FinishedFunc != nullptr) {
                if (interChipA_MDMA_FinishedFunc[i]() == 0) {
                    interChipA_MDMA_FinishedFunc[i] = nullptr;
                    interChipA_MDMA_Finished[i] = 0;
                }
            }
            else {
                interChipA_MDMA_Finished[i] = 0;
            }
        }
        if (interChipA_DMA_Finished) {
            if (interChipA_DMA_FinishedFunc != nullptr) {
                if (interChipA_DMA_FinishedFunc[i]() == 0) {
                    interChipA_DMA_FinishedFunc[i] = nullptr;
                    interChipA_DMA_Finished[i] = 0;
                }
            }
            else {
                interChipA_DMA_Finished[i] = 0;
            }
        }
        if (interChipB_MDMA_Finished) {
            if (interChipB_MDMA_FinishedFunc != nullptr) {
                if (interChipB_MDMA_FinishedFunc[i]() == 0) {
                    interChipB_MDMA_FinishedFunc[i] = nullptr;
                    interChipB_MDMA_Finished[i] = 0;
                }
            }
            else {
                interChipB_MDMA_Finished[i] = 0;
            }
        }
        if (interChipB_DMA_Finished) {
            if (interChipB_DMA_FinishedFunc != nullptr) {
                if (interChipB_DMA_FinishedFunc[i]() == 0) {
                    interChipB_DMA_FinishedFunc[i] = nullptr;
                    interChipB_DMA_Finished[i] = 0;
                }
            }
            else {
                interChipB_DMA_Finished[i] = 0;
            }
        }
    }

    // InterChip send flags
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
}

}; // namespace FlagHandler
