#ifdef POLYCONTROL
#pragma once

#include "circularbuffer/circularbuffer.hpp"

void comReceive(uint8_t *Buf, uint32_t *Len);

uint8_t comAvailable();
char comRead();

#endif