#include "polyRender.hpp"

// Buffer for InterChip Com
RAM2_DMA volatile uint8_t interChipDMABuffer[2 * INTERCHIPBUFFERSIZE];

/// Layer
ID layerId;
Layer layerA(layerId.getNewId());

// InterChip Com
COMinterChip layerCom;

MCP4728 cvDacA = MCP4728(&hi2c1, 0x00, LDAC_1_GPIO_Port, LDAC_1_Pin);
MCP4728 cvDacB = MCP4728(&hi2c1, 0x01, LDAC_2_GPIO_Port, LDAC_2_Pin);
MCP4728 cvDacC = MCP4728(&hi2c1, 0x02, LDAC_3_GPIO_Port, LDAC_3_Pin);

void hardwareInit() {

    // updateI2CAddress(); // update the I2C addresses of the MCP4728 DACs

    cvDacA.init();
    cvDacB.init();
    cvDacC.init();
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    // InterChip Com SPI 1
    if (hspi == &hspi1) {
        if (FlagHandler::interChipReceive_DMA_Started == 1) {
            FlagHandler::interChipReceive_DMA_Started = 0;
            FlagHandler::interChipReceive_DMA_Finished = 1;
        }
    }
}

uint16_t cvDacAbuffer[4];
uint16_t cvDacBbuffer[4];
uint16_t cvDacCbuffer[4];

void PolyRenderInit() {

    // disable reception line
    HAL_GPIO_WritePin(SPI_Ready_toControl_GPIO_Port, SPI_Ready_toControl_Pin, GPIO_PIN_RESET);

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

    // init allLayers
    allLayers.push_back(&layerA);

    layerCom.initInTransmission(
        std::bind<uint8_t>(HAL_SPI_Transmit_DMA, &hspi1, std::placeholders::_1, std::placeholders::_2),
        std::bind<uint8_t>(HAL_SPI_DMAStop, &hspi1), (uint8_t *)interChipDMABuffer);

    layerCom.beginReceiveTransmission();
}

void PolyRenderRun() {
    while (1) {

        FlagHandler::handleFlags();

        static uint32_t timer = __HAL_TIM_GetCounter(&htim2);

        if (__HAL_TIM_GetCounter(&htim2) - timer > 100) {
            timer = __HAL_TIM_GetCounter(&htim2);
            cvDacA.fastUpdate();
            cvDacB.fastUpdate();
            cvDacC.fastUpdate();

            cvDacAbuffer[0] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, powf(2, (float)cvDacAbuffer[0] / (float)409));
            __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, cvDacAbuffer[0] / 4);
            cvDacAbuffer[1] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 300000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, powf(2, (float)cvDacAbuffer[1] / (float)409));
            // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, powf(2, (float)cvDacAbuffer[1] / (float)409));

            cvDacAbuffer[2] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 500000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_3, powf(2, (float)cvDacAbuffer[2] / (float)409));
            cvDacAbuffer[3] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 700000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, powf(2, (float)cvDacAbuffer[3] / (float)409));

            cvDacBbuffer[0] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, powf(2, (float)cvDacBbuffer[0] / (float)409));
            cvDacBbuffer[1] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            // __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, powf(2, (float)cvDacBbuffer[1] / (float)409));
            __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_2, powf(2, (float)cvDacBbuffer[1] / (float)409));

            cvDacBbuffer[2] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            cvDacBbuffer[3] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;

            cvDacCbuffer[0] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            cvDacCbuffer[1] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            cvDacCbuffer[2] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
            cvDacCbuffer[3] = fast_sin_f32((float)__HAL_TIM_GetCounter(&htim2) / 1000000.0f) * 2048 + 2047;
        }

        // HAL_Delay(1);

        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 100);
        // HAL_Delay(50);
        // __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1);
    }
}

// EXTI Callbacks
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    // reception Line from Control
    if (pin == GPIO_PIN_8)
        FlagHandler::interChipReceive_DMA_Finished = true;
}