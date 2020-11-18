#pragma once

// #include "mdma.h"
// #include "spi.h"
// #include <functional>

#include "poly.hpp"

void PolyRenderInit();
void PolyRenderRun();

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
