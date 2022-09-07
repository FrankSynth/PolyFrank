
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_it.h"
#include "com/com.hpp"
#include "main.h"

extern MDMA_HandleTypeDef hmdma_mdma_channel40_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel41_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel42_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel43_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel44_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel45_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel46_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel47_sw_0;
extern MDMA_HandleTypeDef hmdma_mdma_channel48_sw_0;
extern DMA_HandleTypeDef hdma_sai1_a;
extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern DMA_HandleTypeDef hdma_spi4_rx;
extern DMA_HandleTypeDef hdma_spi4_tx;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi4;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern UART_HandleTypeDef huart1;

void NMI_Handler(void) {
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1) {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}
void HardFault_Handler(void) {

    while (1) {
    }
}

void MemManage_Handler(void) {

    while (1) {
    }
}

void BusFault_Handler(void) {

    while (1) {
    }
}

void UsageFault_Handler(void) {

    while (1) {
    }
}

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void) {

    HAL_IncTick();
}

void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}

/**
 * @brief This function handles DMA1 stream2 global interrupt.
 */
void DMA1_Stream2_IRQHandler(void) {
    /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */

    /* USER CODE END DMA1_Stream2_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart1_rx);
    /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

    /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream3 global interrupt.
 */
void DMA1_Stream3_IRQHandler(void) {
    /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */

    /* USER CODE END DMA1_Stream3_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
    /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

    /* USER CODE END DMA1_Stream3_IRQn 1 */
}
void SAI1_IRQHandler(void) {
    HAL_SAI_IRQHandler(&hsai_BlockA1);
}
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void TIM15_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim15);
}

void TIM16_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim16);
}

void SPI1_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi1);
}
void SPI2_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi2);
}

void SPI4_IRQHandler(void) {
    HAL_SPI_IRQHandler(&hspi4);
}

/**
 * @brief This function handles USART1 global interrupt.
 */
void USART1_IRQHandler(void) {
    /* USER CODE BEGIN USART1_IRQn 0 */

    /* USER CODE END USART1_IRQn 0 */

    // RESTART UART
    HAL_UART_IRQHandler(&huart1); /* USER CODE BEGIN USART1_IRQn 1 */

    /* USER CODE END USART1_IRQn 1 */
}
void MDMA_IRQHandler(void) {
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel40_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel41_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel42_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel43_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel44_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel45_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel46_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel47_sw_0);
    HAL_MDMA_IRQHandler(&hmdma_mdma_channel48_sw_0);
}

void DMA1_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_sai1_a);
}

void DMA2_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

void DMA2_Stream2_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi2_tx);
}

void DMA2_Stream5_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi4_rx);
}

void DMA2_Stream6_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_spi4_tx);
}
