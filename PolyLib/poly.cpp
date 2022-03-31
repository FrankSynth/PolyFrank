#include "poly.hpp"
#include "modules/moduleCallbacks.hpp"

std::vector<Layer *> allLayers;

/**
 * @brief init for both Control and Render
 *
 */
void initPoly() {

    // prep lin2log
    precomputeNoteLin2LogTable();

    // set all Flags to default state
    FlagHandler::initFlagHandler();

    // load knob changed callbacks
    setModuleCallbacks();
}

void comMDMACallbackError(MDMA_HandleTypeDef *_hmdma) {
    println("MDMA ERROR");
}
