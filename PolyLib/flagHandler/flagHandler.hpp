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

extern bool saiHalfCptl;
extern std::function<uint8_t()> saiHalfCptlFunc;
extern bool saiCptl;
extern std::function<uint8_t()> saiCptlFunc;

#endif

void handleFlags();
void initFlagHandler();

}; // namespace FlagHandler
