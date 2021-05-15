#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "hardware/MCP4728.hpp"
#include "hardware/PCM1690.hpp"
#include "hardware/TS3A5017D.hpp"
#include "i2c.h"
#include "main.h"
#include "math/polyMath.hpp"
#include "poly.hpp"
#include "render/renderAudio.hpp"
#include "render/renderCV.hpp"
#include "tim.h"

extern i2cpin i2c1Pins;



void PolyRenderInit();
void PolyRenderRun();
