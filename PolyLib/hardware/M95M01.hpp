#pragma once

#include "hardware/bus.hpp"
#include "hardware/driver.hpp"

/* M95040 SPI EEPROM defines */
#define EEPROM_WREN 0x06  /*!< Write Enable */
#define EEPROM_WRDI 0x04  /*!< Write Disable */
#define EEPROM_RDSR 0x05  /*!< Read Status Register */
#define EEPROM_WRSR 0x01  /*!< Write Status Register */
#define EEPROM_READ 0x03  /*!< Read from Memory Array */
#define EEPROM_WRITE 0x02 /*!< Write to Memory Array */

#define EEPROM_WIP_FLAG 0x01 /*!< Write In Progress (WIP) flag */

#define EEPROM_PAGESIZE 256    /*!< Pagesize according to documentation */
#define EEPROM_BUFFER_SIZE 256 /*!< EEPROM Buffer size. Setup to your needs */

class M95M01 : public baseDevice {

  public:
    void configurate(spiBus *busInterface, GPIO_TypeDef *gpioPort, uint16_t gpioPin) {
        this->busInterface = busInterface;

        this->gpioPin = gpioPin;
        this->gpioPort = gpioPort;

        deviceName = "M95M01";
        state = DEVICE_READY;
    }

    void SPI_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
    void SPI_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
    void SPI_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);

    uint8_t SPI_WaitStandbyState(void);

    void SPI_SendInstruction(uint8_t *instruction, uint8_t size);
    void SPI_ReadStatusByte(SPI_HandleTypeDef SPIe, uint8_t *statusByte);

    /* Low layer functions */
    uint8_t SendByte(uint8_t byte);
    void sEE_WriteEnable(void);
    void sEE_WriteDisable(void);
    void sEE_WriteStatusRegister(uint8_t regval);
    uint8_t sEE_ReadStatusRegister(void);

  private:
    spiBus *busInterface;
    uint8_t EEPROM_StatusByte;
    uint8_t RxBuffer[EEPROM_BUFFER_SIZE] = {0x00};

    GPIO_TypeDef *gpioPort;
    uint16_t gpioPin;
};
