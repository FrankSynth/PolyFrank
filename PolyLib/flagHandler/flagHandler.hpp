#pragma once

#include <functional>

namespace FlagHandler {

#ifdef POLYCONTROL

// InterChip send flags
extern bool interChipA_MDMA_Started[2];
extern bool interChipA_MDMA_Finished[2];
extern std::function<uint8_t()> interChipA_MDMA_FinishedFunc[2];
extern bool interChipA_DMA_Started[2];
extern bool interChipA_DMA_Finished[2];
extern std::function<uint8_t()> interChipA_DMA_FinishedFunc[2];
extern bool interChipB_MDMA_Started[2];
extern bool interChipB_MDMA_Finished[2];
extern std::function<uint8_t()> interChipB_MDMA_FinishedFunc[2];
extern bool interChipB_DMA_Started[2];
extern bool interChipB_DMA_Finished[2];
extern std::function<uint8_t()> interChipB_DMA_FinishedFunc[2];

extern bool Control_Encoder_Interrupt;
extern std::function<void()> Control_Encoder_ISR;

extern bool Control_Touch_Interrupt;
extern std::function<void()> Control_Touch_ISR;

extern bool Panel_0_Touch_Interrupt;
extern std::function<void()> Panel_0_Touch_ISR;

extern bool Panel_1_Touch_Interrupt;
extern std::function<void()> Panel_1_Touch_ISR;

extern bool Panel_0_EOC_Interrupt;
extern std::function<void()> Panel_0_EOC_ISR;

extern bool Panel_1_EOC_Interrupt;
extern std::function<void()> Panel_1_EOC_ISR;

// Display
extern bool renderingDoneSwitchBuffer;

#elif POLYRENDER

// InterChip receive flags
extern bool interChipReceive_DMA_Started;
extern bool interChipReceive_DMA_Finished;
extern std::function<uint8_t()> interChipReceive_DMA_FinishedFunc;
extern bool interChipReceive_MDMA_Started;
extern bool interChipReceive_MDMA_Finished;
extern std::function<uint8_t()> interChipReceive_MDMA_FinishedFunc;

// extern bool saiHalfCptl;
// extern std::function<uint8_t()> saiHalfCptlFunc;
// extern bool saiCptl;
// extern std::function<uint8_t()> saiCptlFunc;

extern bool cvDacAStarted;
extern bool cvDacBStarted;
extern bool cvDacCStarted;
extern bool cvDacCFinished;

extern bool renderNewCV;
extern std::function<void()> renderNewCVFunc;

#endif

void handleFlags();
void initFlagHandler();

}; // namespace FlagHandler
