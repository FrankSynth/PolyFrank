#include "poly.hpp"

std::vector<Layer *> allLayers;

void initPoly() {
    // set all Flags to default state
    FlagHandler::initFlagHandler();

    // prep lin2log
    precomputeNoteLin2LogTable();

    // init global settings if necessary
}
