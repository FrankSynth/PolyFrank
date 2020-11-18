#include "polyRender.hpp"
#include "hardware/MCP4728.hpp"
#include "i2c.h"
#include "tim.h"

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABuffer[2 * INTERCHIPBUFFERSIZE];

/// Layer
ID layerId;
// Layer layerA(layerId.getNewId());

// layerACom.initOutTransmission(std::bind<uint8_t>(HAL_SPI_Receive_DMA, &hspi1, std::placeholders::_1,
//  std::placeholders::_2));

// InterChip Com
// COMinterChip layerCom;

MCP4728 cvDacA = MCP4728(&hi2c1, 0x01, LDAC_1_GPIO_Port, LDAC_1_Pin);
MCP4728 cvDacB = MCP4728(&hi2c1, 0x02, LDAC_2_GPIO_Port, LDAC_2_Pin);
MCP4728 cvDacC = MCP4728(&hi2c1, 0x03, LDAC_3_GPIO_Port, LDAC_3_Pin);

void hardwareInit() {

    updateI2CAddress(); // update the I2C addresses of the MCP4728 DACs

    cvDacA.init();
    cvDacB.init();
    cvDacC.init();
}

// void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

//     // InterChip Com SPI 1
//     if (hspi == &hspi1) {
//         if (FlagHandler::interChipReceive_DMA_Started == 1) {
//             FlagHandler::interChipReceive_DMA_Started = 0;
//             FlagHandler::interChipReceive_DMA_Finished = 1;
//         }
//     }
// }

uint16_t cvDacAbuffer[4];
uint16_t cvDacBbuffer[4];
uint16_t cvDacCbuffer[4];

void PolyRenderInit() {
    hardwareInit();

    cvDacA.setDataPointer((uint8_t *)cvDacAbuffer);
    cvDacB.setDataPointer((uint8_t *)cvDacBbuffer);
    cvDacC.setDataPointer((uint8_t *)cvDacCbuffer);

    cvDacAbuffer[0] = 1000;
    cvDacAbuffer[1] = 2000;
    cvDacAbuffer[2] = 3000;
    cvDacAbuffer[3] = 4000;

    cvDacBbuffer[0] = 1000;
    cvDacBbuffer[1] = 2000;
    cvDacBbuffer[2] = 3000;
    cvDacBbuffer[3] = 4000;

    cvDacCbuffer[0] = 1000;
    cvDacCbuffer[1] = 2000;
    cvDacCbuffer[2] = 3000;
    cvDacCbuffer[3] = 4000;

    // allLayers.push_back(&layerA);

    // layerCom.initInTransmission(
    //     std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
    //     std::bind<uint8_t>(HAL_SPI_DMAStop, &hspi1), (uint8_t *)interChipDMABuffer);

    // layerCom.beginReceiveTransmission();
}

void PolyRenderRun() {
    while (1) {
        cvDacA.fastUpdate();
        cvDacB.fastUpdate();
        cvDacC.fastUpdate();

        HAL_Delay(100);

        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 100);
        HAL_Delay(1000);
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1);
    }
}