#pragma once
#include "datacore/dataHelperFunctions.hpp"
#include "flagHandler/flagHandler.hpp"
#include "hardware/MCP4728.hpp"
#include "layer/layer.hpp"
#include "math/polyMath.hpp"
#include "tim.h"
#include <stdint.h>

void initCVRendering();
void renderCVs();