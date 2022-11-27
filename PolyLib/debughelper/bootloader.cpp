#include "debughelper/bootloader.hpp"
#include <string.h>

///////////https://github.com/joeferner/stm32-spi-bootloader/blob/master/src/spi-bootloader.c

SPI_HandleTypeDef *SpiHandle;

extern void ISP_ErrorHandler();

static uint8_t xor_checksum(const uint8_t pData[], uint8_t len);

/**
 * @brief  Initialize the SPI Bootloader host with the associated SPI handle
 *         and synchronize with target.
 * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
 *               the configuration information for SPI module.
 * @retval None
 */
void BL_Init(SPI_HandleTypeDef *hspi, uint32_t layer, uint32_t chip) {
    uint8_t sync_byte = BL_SPI_SOF;
    uint8_t receive_byte;

    /* Associate SPI handle */
    SpiHandle = hspi;

    /* Send synchronization Byte */

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(SpiHandle, &sync_byte, &receive_byte, 1U, 1000U);

    if (receive_byte != 0xA5) {
        return;
    }

    /* Get SYNC Byte ACK*/
    wait_for_ack_WithoutCS();

    setCSLine(layer, chip, GPIO_PIN_SET);
}

/**
 * @brief  Read data from any valid memory address
 * @param  address: start address
 * @param  nob: number of bytes to be read
 * @param  pData: pointer to destination data buffer
 * @retval None
 */
void BL_ReadMemory_Command(uint32_t address, uint8_t nob, uint8_t *pData, uint32_t layer, uint32_t chip) {
    uint8_t cmd_frame[3];
    uint8_t addr_frame[5];
    uint8_t nob_frame[2];
    uint8_t dummy = 0x00U;

    /* Send start of frame (0x5A) + Read Memory command frame (0x11 0xEE) */
    cmd_frame[0] = BL_SPI_SOF;
    cmd_frame[1] = RMEM_COMMAND;
    cmd_frame[2] = RMEM_COMMAND ^ 0xFFU;

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, cmd_frame, 3U, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);

    /* Wait for ACK or NACK frame */
    wait_for_ack(layer, chip);

    /* Send data frame: start address (4 Bytes) + Checksum (1 Byte) */
    addr_frame[0] = (uint8_t)((address >> 24) & 0xFFU);
    addr_frame[1] = (uint8_t)((address >> 16) & 0xFFU);
    addr_frame[2] = (uint8_t)((address >> 8) & 0xFFU);
    addr_frame[3] = (uint8_t)(address & 0xFFU);
    addr_frame[4] = xor_checksum(addr_frame, 4U);

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, addr_frame, 5U, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);

    /* Wait for ACK or NACK frame */
    wait_for_ack(layer, chip);

    /* Send data frame: number of Bytes to be read (1 Byte) + checksum (1 Byte) */
    nob_frame[0] = (nob - 1U);
    nob_frame[1] = (nob - 1U) ^ 0xFFU;

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, nob_frame, 2U, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);

    /* Wait for ACK or NACK frame */
    wait_for_ack(layer, chip);

    /* Receive data frame: data from the Bootloader */
    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, &dummy, 1U, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Receive(SpiHandle, pData, (uint16_t)nob, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);
}

/**
 * @brief  Write data to any valid memory address
 * @param  address: start address
 * @param  nob: number of bytes to be written
 * @param  pData: pointer to source data pointer
 * @retval None
 */
void BL_WriteMemory_Command(uint32_t address, uint8_t nob, uint8_t *pData, uint32_t layer, uint32_t chip) {
    uint8_t cmd_frame[3];
    uint8_t addr_frame[5];
    uint8_t n = nob - 1U;
    uint8_t checksum = xor_checksum(pData, nob) ^ n;

    /* Send start of frame (0x5A) + Write Memory command frame (0x21 0xDE) */
    cmd_frame[0] = BL_SPI_SOF;
    cmd_frame[1] = WMEM_COMMAND;
    cmd_frame[2] = WMEM_COMMAND ^ 0xFFU;

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, cmd_frame, 3U, 1000U);

    /* Wait for ACK or NACK frame */
    wait_for_ack_WithoutCS();

    /* Send data frame: Write Memory start address (4 Bytes) + Checksum (1 Byte) */
    addr_frame[0] = ((uint8_t)(address >> 24) & 0xFFU);
    addr_frame[1] = ((uint8_t)(address >> 16) & 0xFFU);
    addr_frame[2] = ((uint8_t)(address >> 8) & 0xFFU);
    addr_frame[3] = ((uint8_t)address & 0xFFU);
    addr_frame[4] = xor_checksum(addr_frame, 4U);

    HAL_SPI_Transmit(SpiHandle, addr_frame, 5U, 1000U);

    /* Wait for ACK or NACK frame */
    wait_for_ack_WithoutCS();

    /* Send data frame: N number of Bytes to be written (1 Byte),
       N + 1 data Bytes and a checksum (1 Byte) */

    HAL_SPI_Transmit(SpiHandle, &n, 1U, 1000U);
    HAL_SPI_Transmit(SpiHandle, pData, (uint16_t)nob, 1000U);
    HAL_SPI_Transmit(SpiHandle, &checksum, 1U, 1000U);

    setCSLine(layer, chip, GPIO_PIN_SET);

    /* Wait for ACK or NACK frame */
    // wait_for_ack(layer, chip);
}

/**
 * @brief  Erase one or more Flash memory pages or sectors
 * @param  nb: number of pages or sectors to be erased
 *         This parameter can also be one of the special erase values:
 *           @arg @ref ERASE_ALL for a global mass erase
 *           @arg @ref ERASE_BANK1 for a Bank 1 mass erase (only for products supporting this feature)
 *           @arg @ref ERASE_BANK2 for a Bank 2 mass erase (only for products supporting this feature)
 *         Values from 0xFFFC to 0xFFF0 are reserved
 * @param  code: memory page or sector
 * @retval None
 */
void BL_EraseMemory_Command(uint16_t nb, uint8_t code, uint32_t layer, uint32_t chip) {
    uint8_t cmd_frame[3];
    uint8_t data_frame[3];

    /* Send start of frame (0x5A) + Erase Memory command frame (0x44 0xBB) */
    cmd_frame[0] = BL_SPI_SOF;
    cmd_frame[1] = 0x44;
    cmd_frame[2] = 0xBB;

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, cmd_frame, 3U, 1000U);

    /* Wait for ACK or NACK frame */
    wait_for_ack_WithoutCS();

    /* Send data frame: nb (2 Bytes), the number of pages or sectors to be
       erased + checksum (1 Byte)*/
    data_frame[0] = (uint8_t)(nb >> 8) & 0xFFU;
    data_frame[1] = (uint8_t)nb & 0xFFU;
    data_frame[2] = data_frame[0] ^ data_frame[1];

    HAL_SPI_Transmit(SpiHandle, data_frame, 3U, 1000U);

    /* Wait for ACK or NACK frame */
    wait_for_ack_WithoutCS();

    /* If non-special erase, send page number (2 Bytes) + checksum (1 Byte)*/
    if ((nb >> 4) != 0xFFF) {
        data_frame[0] = (uint8_t)(code >> 8) & 0xFFU;
        data_frame[1] = code & 0xFFU;
        data_frame[2] = data_frame[0] ^ data_frame[1];

        HAL_SPI_Transmit(SpiHandle, data_frame, 3U, 1000U);

        /* Wait for ACK or NACK frame */
        wait_for_ack_WithoutCS();
    }

    setCSLine(layer, chip, GPIO_PIN_SET);
}

void BL_MassErase_Command_all() {

    uint8_t cmd_frame[3];
    uint8_t data_frame[3];

    /* Send start of frame (0x5A) + Erase Memory command frame (0x44 0xBB) */
    cmd_frame[0] = BL_SPI_SOF;
    cmd_frame[1] = 0x44;
    cmd_frame[2] = 0xBB;

    /* Send data frame: nb (2 Bytes), the number of pages or sectors to be
       erased + checksum (1 Byte)*/
    data_frame[0] = 0xFF;
    data_frame[1] = 0xFF;
    data_frame[2] = data_frame[0] ^ data_frame[1];

    BL_Send_CommandACK(3u, cmd_frame, 0, 0);
    HAL_Delay(1);

    BL_Send_Command(3u, data_frame, 0, 0);
    HAL_Delay(1);

    BL_Send_CommandACK(3u, cmd_frame, 0, 1);
    HAL_Delay(1);

    BL_Send_Command(3u, data_frame, 0, 1);
    HAL_Delay(1);

    BL_Send_CommandACK(3u, cmd_frame, 1, 0);
    HAL_Delay(1);

    BL_Send_Command(3u, data_frame, 1, 0);
    HAL_Delay(1);

    BL_Send_CommandACK(3u, cmd_frame, 1, 1);
    HAL_Delay(1);

    BL_Send_Command(3u, data_frame, 1, 1);
    HAL_Delay(1);

    wait_for_ack(0, 0);
    wait_for_ack(0, 1);
    wait_for_ack(1, 0);
    wait_for_ack(1, 1);
}

void BL_Send_Command(uint16_t nb, uint8_t *data, uint32_t layer, uint32_t chip) {

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, data, nb, 1000U);
    setCSLine(layer, chip, GPIO_PIN_SET);
}

void BL_Send_CommandACK(uint16_t nb, uint8_t *data, uint32_t layer, uint32_t chip) {

    setCSLine(layer, chip, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SpiHandle, data, nb, 1000U);
    /* Wait for ACK or NACK frame */
    wait_for_ack_WithoutCS();
    setCSLine(layer, chip, GPIO_PIN_SET);
}

void wait_for_ack(uint32_t layer, uint32_t chip) {
    uint8_t resp;
    uint8_t dummy = 0x00U;
    uint8_t ack = BL_ACK;
    uint32_t ack_received = 0U;

    setCSLine(layer, chip, GPIO_PIN_RESET);

    while (ack_received != 1U) {

        HAL_SPI_TransmitReceive(SpiHandle, &dummy, &resp, 1U, 1000U);

        if (resp == BL_ACK) {
            /* Received ACK: send ACK */

            HAL_SPI_Transmit(SpiHandle, &ack, 1U, 1000U);

            ack_received = 1U;
        }
        else if (resp == BL_NAK) {
            /* Received NACK */
            ISP_ErrorHandler();
        }
        else {

            /* Received junk */
        }
    }
    setCSLine(layer, chip, GPIO_PIN_SET);
}

void wait_for_ack_WithoutCS() {
    uint8_t resp;
    uint8_t dummy = 0x00U;
    uint8_t ack = BL_ACK;
    uint32_t ack_received = 0U;

    while (ack_received != 1U) {

        HAL_SPI_TransmitReceive(SpiHandle, &dummy, &resp, 1U, 1000U);

        if (resp == BL_ACK) {
            /* Received ACK: send ACK */

            HAL_SPI_Transmit(SpiHandle, &ack, 1U, 1000U);

            ack_received = 1U;
        }
        else if (resp == BL_NAK) {
            /* Received NACK */
            ISP_ErrorHandler();
        }
        else {

            /* Received junk */
        }
    }
}

void sendDummy() {
    uint8_t dummy = 0x00U;

    HAL_SPI_Transmit(SpiHandle, &dummy, 1U, 1000U);
}

static uint8_t xor_checksum(const uint8_t pData[], uint8_t len) {
    uint8_t sum = *pData;

    for (uint8_t i = 1U; i < len; i++) {
        sum ^= pData[i];
    }

    return sum;
}
