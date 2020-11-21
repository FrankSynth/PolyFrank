#pragma once

// FIXME move all includes to cpp files when possible
#include "com/com.hpp"
#include "debughelper/debughelper.hpp"
#include "flagHandler/flagHandler.hpp"
#include "layer/layer.hpp"

#define LEDMAXBRIGHTNESSCOUNT 1024 // match pwm counter value

extern std::vector<Layer *> allLayers;

void initPoly();
