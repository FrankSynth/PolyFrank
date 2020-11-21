#include "poly.hpp"

std::vector<Layer *> allLayers;

void initPoly() {
    // set all Flags to default state

    // prep lin2log
    precomputeNoteLin2LogTable();

    FlagHandler::initFlagHandler();
    // init global settings if necessary
}
