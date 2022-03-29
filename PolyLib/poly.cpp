#include "poly.hpp"
#include "modules/moduleCallbacks.hpp"

std::vector<Layer *> allLayers;

/**
 * @brief init for both Control and Render
 *
 */
void initPoly() {

    // set all Flags to default state

    // prep lin2log
    precomputeNoteLin2LogTable();

    FlagHandler::initFlagHandler();
    // init global settings if necessary
    setModuleCallbacks();
}

void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma) {
    println("MDMA ERROR");
}

// void comMDMACallback(MDMA_HandleTypeDef *_hmdma) {

//     HAL_MDMA_UnRegisterCallback(&hmdma_mdma_channel40_sw_0, HAL_MDMA_XFER_CPLT_CB_ID);

//     if (FlagHandler::interChipSend_MDMA_Started == 1) {
//         FlagHandler::interChipSend_MDMA_Started = 0;
//         FlagHandler::interChipSend_MDMA_Finished = 1;
//     }
// }