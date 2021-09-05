#ifdef POLYCONTROL

#include "PCA9548.hpp"

PCA9548 i2cBusSwitchLayer[2] = {PCA9548(&hi2c4, 0x00), PCA9548(&hi2c3, 0x00)};
PCA9548 i2cBusSwitchControl = PCA9548(&hi2c1, 0x00);

#endif