#ifdef POLYCONTROL

#include "debughelper/firmware.hpp"
#include "debughelper/bootloader.hpp"
#include "debughelper/debughelper.hpp"
#include "main.h"
#include "spi.h"

// TODO WATCHDOG FOR BOOLOADER RESTART

///////////////////ISP///////////

typedef struct {
    uint16_t blocksize = 0x00;
    uint16_t blockcount = 0x00;
    uint8_t targetID = 0x00;
} SETTINGSBLOCK;

uint8_t checkDataValid() { // wait for data
    uint8_t validation;

    while (!comAvailable())
        ;

    validation = comRead();
    if (validation == DATAVALID)
        return 1;
    return 0;
}
void flashRenderMCUSPI() {
    uint8_t response;

    response = ENTERMODE;
    CDC_Transmit_FS(&response, 1); // READY FOR DATA

    /////////Clear Polyfrank
    __disable_irq();

    HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

    HAL_NVIC_DisableIRQ(DMA2D_IRQn);

    HAL_NVIC_DisableIRQ(EXTI2_IRQn);
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);

    HAL_NVIC_DisableIRQ(LTDC_IRQn);
    HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);

    // HAL_NVIC_DisableIRQ(SPI1_IRQn);
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
    HAL_NVIC_DisableIRQ(SPI6_IRQn);

    HAL_NVIC_DisableIRQ(UART5_IRQn);

    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
    HAL_NVIC_DisableIRQ(TIM16_IRQn);

    __enable_irq();

    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_RESET);

    /////////INIT Hardware
    HAL_SPI_MspDeInit(&hspi1);
    hspi1.State = HAL_SPI_STATE_RESET; // need to be in reset state to reconfigurate hardware pins
    // __SPI1_FORCE_RESET();
    MX_SPI1_BOOLOADER_Init();

    // clear CS lines
    setCSLine(0, 0, GPIO_PIN_SET);
    setCSLine(1, 0, GPIO_PIN_SET);
    setCSLine(0, 1, GPIO_PIN_SET);
    setCSLine(1, 1, GPIO_PIN_SET);

    // START Bootloader
    HAL_Delay(100);
    HAL_GPIO_WritePin(Layer_Boot_GPIO_Port, Layer_Boot_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    /////////INIT Bootloader SPI

    // Init all devices
    BL_Init(&hspi1, 0, 0);
    BL_Init(&hspi1, 0, 1);
    BL_Init(&hspi1, 1, 0);
    BL_Init(&hspi1, 1, 1);

    HAL_Delay(10);

    /////////Erase all
    BL_MassErase_Command_all();

    response = ERASEDONE;
    CDC_Transmit_FS(&response, 1); // READY FOR DATA

    /////////Flash Info
    SETTINGSBLOCK settings;

    for (uint32_t i = 0; i < 5; i++) // read 5 byte
    {
        while (!comAvailable())
            ; // wait for data

        ((uint8_t *)&settings)[i] = comRead();
    }
    response = DATAVALID;

    CDC_Transmit_FS(&response, 1); // Response data valid

    /////////START Flashing

    uint8_t *blockbuffer = new uint8_t[settings.blocksize];

    for (uint32_t block = 0; block < settings.blockcount; block++) // read all blocks
    {
        for (uint32_t byteIndex = 0; byteIndex < settings.blocksize; byteIndex++) // read all bytes
        {
            while (!comAvailable())
                ; // wait for data

            blockbuffer[byteIndex] = comRead();
        }
        CDC_Transmit_FS(&response, 1); // Ready for new data

        // Send data via SPI
        ///// WRITE MEMORY LAST ACK DEACTIVATED!!!
        BL_WriteMemory_Command(0x08000000 + (block * settings.blocksize), (uint8_t)(settings.blocksize), blockbuffer, 0,
                               0); // Write Chip;

        BL_WriteMemory_Command(0x08000000 + (block * settings.blocksize), (uint8_t)(settings.blocksize), blockbuffer, 0,
                               1); // Write Chip;

        BL_WriteMemory_Command(0x08000000 + (block * settings.blocksize), (uint8_t)(settings.blocksize), blockbuffer, 1,
                               0); // Write Chip;

        BL_WriteMemory_Command(0x08000000 + (block * settings.blocksize), (uint8_t)(settings.blocksize), blockbuffer, 1,
                               1); // Write Chip;

        wait_for_ack(0, 0);
        wait_for_ack(0, 1);
        wait_for_ack(1, 0);
        wait_for_ack(1, 1);
    }

    /////////Update Done
    response = UPDATEFINISH;
    CDC_Transmit_FS(&response, 1); // Update finished

    delete blockbuffer;

    /////////Reset System
    HAL_Delay(100);
    HAL_GPIO_WritePin(Layer_Boot_GPIO_Port, Layer_Boot_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);

    NVIC_SystemReset();
}

//////////////////DFU////////////
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

void rebootToBooloader() { // https://github.com/rusefi/rusefi/pull/2999/files
    // // Enable Layer Board
    // HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
    // // Enable Panel Board
    // HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_RESET);
    // // Enable Control Panel Board
    // HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_RESET);

    // /* Disable all interrupts */
    // __disable_irq();

    // /* Clear Interrupt Enable Register & Interrupt Pending Register */
    // for (uint32_t i = 0; i < 8; i++) {
    //     NVIC->ICER[i] = 0xFFFFFFFF;
    //     NVIC->ICPR[i] = 0xFFFFFFFF;
    // }

    // /* Re-enable all interrupts */
    // __enable_irq();

    // HAL_Delay(100);
    // HAL_GPIO_WritePin(Layer_RST_GPIO_Port, Layer_RST_Pin, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(Panel_RST_GPIO_Port, Panel_RST_Pin, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(Control_RST_GPIO_Port, Control_RST_Pin, GPIO_PIN_RESET);

    // __disable_irq();

    // HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
    // HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
    // HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    // HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    // HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

    // HAL_NVIC_DisableIRQ(DMA2D_IRQn);

    // HAL_NVIC_DisableIRQ(EXTI2_IRQn);
    // HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    // HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    // HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    // HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

    // HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    // HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    // HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
    // HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);

    // HAL_NVIC_DisableIRQ(LTDC_IRQn);
    // HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);

    // HAL_NVIC_DisableIRQ(SPI1_IRQn);
    // HAL_NVIC_DisableIRQ(SPI2_IRQn);
    // HAL_NVIC_DisableIRQ(SPI6_IRQn);

    // HAL_NVIC_DisableIRQ(UART5_IRQn);

    // HAL_NVIC_DisableIRQ(TIM3_IRQn);
    // HAL_NVIC_DisableIRQ(TIM4_IRQn);
    // HAL_NVIC_DisableIRQ(TIM16_IRQn);

    // __enable_irq();

    // __HAL_RCC_AHB1_FORCE_RESET();
    // __HAL_RCC_AHB2_FORCE_RESET();
    // __HAL_RCC_AHB3_FORCE_RESET();
    // __HAL_RCC_AHB4_FORCE_RESET();
    // __HAL_RCC_APB1L_FORCE_RESET();
    // __HAL_RCC_APB1H_FORCE_RESET();

    // __HAL_RCC_APB1L_RELEASE_RESET();
    // __HAL_RCC_APB1H_RELEASE_RESET();
    // __HAL_RCC_AHB1_RELEASE_RESET();
    // __HAL_RCC_AHB2_RELEASE_RESET();
    // __HAL_RCC_AHB3_RELEASE_RESET();
    // __HAL_RCC_AHB4_RELEASE_RESET();

    // HAL_Delay(500);

    // RCC->AHB1ENR &= ~(RCC_AHB1ENR_USB1OTGHSEN | RCC_AHB1ENR_USB2OTGFSEN);

    *((unsigned long *)0x2001FFF0) = 0xDEADBEEF; // End of RAM
    NVIC_SystemReset();
}

#endif