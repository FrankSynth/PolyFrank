#ifdef POLYCONTROL

#include "M95M01.hpp"

extern volatile uint8_t presetDMABlockBuffer[];
// extern void PolyControlNonUIRunWithoutSend();

void M95M01::SPI_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite) {
    while (busInterface->hspi->State != HAL_SPI_STATE_READY) {
        // HAL_Delay(1);
    }

    sEE_WriteEnable();

    /*
        We gonna send commands in one packet of 3 bytes
     */
    uint8_t header[4];

    header[0] = EEPROM_WRITE;             // Send "Write to Memory" instruction
    header[1] = (WriteAddr >> 16) & 0xFF; // Send 24-bit address
    header[2] = (WriteAddr >> 8) & 0xFF;  // Send 24-bit address
    header[3] = WriteAddr & 0xFF;         // Send 24-bit address

    // Select the EEPROM: Chip Select low
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    SPI_SendInstruction((uint8_t *)header, 4);

    // fast_copy_f32((uint32_t *)pBuffer, (uint32_t *)presetDMABlockBuffer, (NumByteToWrite + 4) >> 2);
    fast_copy_byte(pBuffer, (uint8_t *)presetDMABlockBuffer, NumByteToWrite);

    while (busInterface->transmit((uint8_t *)presetDMABlockBuffer, NumByteToWrite, true) != BUS_OK) {
    }

    while (busInterface->state != BUS_READY) {
    }

    // Deselect the EEPROM: Chip Select high
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

    // Wait the end of EEPROM writing
    SPI_WaitStandbyState();

    // Disable the write access to the EEPROM
    sEE_WriteDisable();
}

void M95M01::SPI_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite) {
    uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    uint32_t sEE_DataNum = 0;

    Addr = WriteAddr % EEPROM_PAGESIZE;
    count = EEPROM_PAGESIZE - Addr;
    NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
    NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

    if (Addr == 0) {          /* WriteAddr is EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0) { /* NumByteToWrite < EEPROM_PAGESIZE */
            sEE_DataNum = NumByteToWrite;
            SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
        }
        else { /* NumByteToWrite > EEPROM_PAGESIZE */
            while (NumOfPage--) {
                sEE_DataNum = EEPROM_PAGESIZE;
                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                WriteAddr += EEPROM_PAGESIZE;
                pBuffer += EEPROM_PAGESIZE;
            }
            if (NumOfSingle) { // check last packet not size 0
                sEE_DataNum = NumOfSingle;
                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            }
        }
    }
    else {                             /* WriteAddr is not EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0) {          /* NumByteToWrite < EEPROM_PAGESIZE */
            if (NumOfSingle > count) { /* (NumByteToWrite + WriteAddr) > EEPROM_PAGESIZE */
                temp = NumOfSingle - count;
                sEE_DataNum = count;
                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                WriteAddr += count;
                pBuffer += count;

                sEE_DataNum = temp;
                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            }
            else {
                sEE_DataNum = NumByteToWrite;
                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            }
        }
        else { /* NumByteToWrite > EEPROM_PAGESIZE */
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
            NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

            sEE_DataNum = count;

            SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

            WriteAddr += count;
            pBuffer += count;

            while (NumOfPage--) {
                sEE_DataNum = EEPROM_PAGESIZE;

                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

                WriteAddr += EEPROM_PAGESIZE;
                pBuffer += EEPROM_PAGESIZE;
            }

            if (NumOfSingle != 0) {
                sEE_DataNum = NumOfSingle;

                SPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
            }
        }
    }
}

void M95M01::SPI_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead) {

    while (busInterface->hspi->State != HAL_SPI_STATE_READY) {
        // HAL_Delay(1);
    }

    /*
        We gonna send all commands in one packet of 3 bytes
     */
    // We have to split the read Commands in mutliple section because the maximum Readlength ist 2^16 -> 0xFFFF Bytes
    uint8_t nReadCommands = (NumByteToRead >> 16) + 1;
    uint16_t nbBytes;
    uint8_t *pToBuffer;

    for (uint32_t i = 0; i < nReadCommands; i++) {
        if ((NumByteToRead - i * 0xFFFF) > 0xFFFF) {
            nbBytes = 0xFFFF;
        }
        else {
            nbBytes = (uint16_t)(NumByteToRead - i * 0xFFFF);
        }

        if (nbBytes == 0) {
            return;
        }
        pToBuffer = (uint8_t *)pBuffer + 0xFFFF * i;

        uint32_t address = ReadAddr + 0xFFFF * i;

        uint8_t header[4];

        header[0] = EEPROM_READ;            // Send "Read from Memory" instruction
        header[1] = (address >> 16) & 0xFF; // Send 24-bit address
        header[2] = (address >> 8) & 0xFF;  // Send 24-bit address
        header[3] = address & 0xFF;         // Send 24-bit address

        // Select the EEPROM: Chip Select low
        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

        /* Send WriteAddr address byte to read from */
        SPI_SendInstruction(header, 4);

        while (busInterface->receive((uint8_t *)presetDMABlockBuffer, nbBytes, true) != BUS_OK) {
        }
        while (busInterface->state != BUS_READY) {
        }

        // fast_copy_f32((uint32_t *)presetDMABlockBuffer, (uint32_t *)pToBuffer, (nbBytes + 4) >> 2);
        fast_copy_byte((uint8_t *)presetDMABlockBuffer, pToBuffer, nbBytes);

        // Deselect the EEPROM: Chip Select high
        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
    }
}

uint8_t M95M01::SendByte(uint8_t byte) {
    uint8_t answerByte;

    /* Loop while DR register in not empty */
    while (busInterface->hspi->State == HAL_SPI_STATE_RESET) {
        // HAL_Delay(1);
    }

    /* Send byte through the SPI peripheral */
    while (busInterface->transmit(&byte, 1) != BUS_OK) {
    }

    /* Wait to receive a byte */
    while (busInterface->hspi->State == HAL_SPI_STATE_RESET) {
        // HAL_Delay(1);
    }

    /* Return the byte read from the SPI bus */
    while (busInterface->receive(&answerByte, 1) != BUS_OK) {
    }

    return (uint8_t)answerByte;
}

void M95M01::sEE_WriteEnable(void) {
    // Select the EEPROM: Chip Select low
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    uint8_t command[1] = {EEPROM_WREN};
    /* Send "Write Enable" instruction */
    SPI_SendInstruction((uint8_t *)command, 1);

    // Deselect the EEPROM: Chip Select high
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
}

void M95M01::sEE_WriteDisable(void) {
    // Select the EEPROM: Chip Select low
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    uint8_t command[1] = {EEPROM_WRDI};

    /* Send "Write Disable" instruction */
    SPI_SendInstruction((uint8_t *)command, 1);

    // Deselect the EEPROM: Chip Select high
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
}

void M95M01::sEE_WriteStatusRegister(uint8_t regval) {
    uint8_t command[2];

    command[0] = EEPROM_WRSR;
    command[1] = regval;

    // Enable the write access to the EEPROM
    sEE_WriteEnable();

    // Select the EEPROM: Chip Select low
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    // Send "Write Status Register" instruction
    // and Regval in one packet
    SPI_SendInstruction((uint8_t *)command, 2);

    // Deselect the EEPROM: Chip Select high
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

    sEE_WriteDisable();
}

uint8_t M95M01::SPI_WaitStandbyState(void) {
    uint8_t sEEstatus[1] = {0x00};
    uint8_t command[1] = {EEPROM_RDSR};

    // Select the EEPROM: Chip Select low
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    // Send "Read Status Register" instruction
    SPI_SendInstruction((uint8_t *)command, 1);

    // Loop as long as the memory is busy with a write cycle
    do {
        while (busInterface->receive((uint8_t *)sEEstatus, 1) != BUS_OK) {
        }
        HAL_Delay(1);
    } while ((sEEstatus[0] & EEPROM_WIP_FLAG) == SET); // Write in progress

    // Deselect the EEPROM: Chip Select high
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);

    return 0;
}

void M95M01::SPI_SendInstruction(uint8_t *instruction, uint8_t size) {
    while (busInterface->hspi->State == HAL_SPI_STATE_RESET) {
        // HAL_Delay(1);
    }

    while (busInterface->transmit((uint8_t *)instruction, (uint16_t)size) != BUS_OK) {
        // HAL_Delay(1);
    }
}

#endif