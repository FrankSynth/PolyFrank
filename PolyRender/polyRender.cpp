#include "polyRender.hpp"
#include "hardware/MCP4728.hpp"
#include "i2c.h"

// Buffer for InterChip Com
// RAM2_DMA uint8_t interChipDMABufferLayerA[2][INTERCHIPBUFFERSIZE];

/// Layer
// ID layerid;

// Layer layer(layerid.getNewId());

// layerACom.initOutTransmission(std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1,
//  std::placeholders::_2));

// InterChip Com

// MCP4728 cvDacA = MCP4728(&hi2c1, 0x01, GPIOF, GPIO_PIN_0);

// hawdwareInit(){
//      cvDacA.init();

//      if first start :
//      updateI2CAddress();  //update the I2C addresses of the MCP4728 DACs
//}
// void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

//     // InterChip Com SPI 1
//     if (hspi == &hspi1) {
//         if (FlagHandler::interChipReceive_DMA_Started == 1) {
//             FlagHandler::interChipReceive_DMA_Started = 0;
//             FlagHandler::interChipReceive_DMA_Finished = 1;
//         }
//     }
// }
