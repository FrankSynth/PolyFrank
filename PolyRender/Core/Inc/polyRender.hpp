#pragma once

#include "hardware/MCP4728.hpp"
#include "i2c.h"
#include "math/polyMath.hpp"
#include "poly.hpp"
#include "tim.h"

void PolyRenderInit();
void PolyRenderRun();

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
