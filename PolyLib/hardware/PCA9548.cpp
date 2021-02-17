#ifdef POLYCONTROL

#include "PCA9548.hpp"

PCA9548 i2cBusSwitch[2] = {PCA9548(&hi2c4, 0x00), PCA9548(&hi2c3, 0x00)};

#endif