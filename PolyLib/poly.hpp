#pragma once

#include "com/com.hpp"
#include "debughelper/debughelper.hpp"
#include "flagHandler/flagHandler.hpp"
#include "globalSettings/globalSettings.hpp"
#include "layer/layer.hpp"

extern GlobalSettings polySettings;

extern std::vector<Layer *> allLayers;

void initPoly();
