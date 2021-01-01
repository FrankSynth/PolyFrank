#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "debughelper/debughelper.hpp"
#include "i2c.h"
#include "tim.h"

// implementation i2C bit-banging : https://en.wikipedia.org/wiki/I%C2%B2C

#define I2CSPEED 10 // 100khz

typedef struct {
    GPIO_TypeDef *portSDA;
    uint16_t pinSDA;
    GPIO_TypeDef *portSCL;
    uint16_t pinSCL;
} i2cpin;

void updateI2CAddress();
void sendI2CAddressUpdate(i2cpin i2cPins, GPIO_TypeDef *latchPort, uint16_t latchPin, uint8_t address);

void writeBit(uint32_t delay, uint8_t data);
void I2C_delay(void);
bool read_SCL(i2cpin pins);  // Return current level of SCL line, 0 or 1
bool read_SDA(i2cpin pins);  // Return current level of SDA line, 0 or 1
void set_SCL(i2cpin pins);   // Do not drive SCL (set pin high-impedance)
void clear_SCL(i2cpin pins); // Actively drive SCL signal low
void set_SDA(i2cpin pins);   // Do not drive SDA (set pin high-impedance)
void clear_SDA(i2cpin pins); // Actively drive SDA signal low
void arbitration_lost(void);

bool i2c_write_byte(i2cpin pins, bool send_start, bool send_stop, unsigned char byte);
unsigned char i2c_read_byte(i2cpin pins, bool nack, bool send_stop);
bool i2c_read_bit(i2cpin pins);
void i2c_write_bit(i2cpin pins, bool bit);
void i2c_stop_cond(i2cpin pins);
void i2c_start_cond(i2cpin pins);

/*
Class for the MCP4728 DAC. It only supports the fast write mode for writing all DAC channel in row.
You have to set the pointer to data first, and then call fastUpdate.

Commands ->   C2 / C1 /C0 / W1 / W2 / DAC1 / DAC0 / UDAC
I2C Address (7bit) ->   1 / 1 /0 / 0 / A2 / A1 / A0 / x

*/

class MCP4728 {
  public:
    MCP4728(I2C_HandleTypeDef *i2cHandle, uint8_t i2cAddress, GPIO_TypeDef *latchPort, uint16_t latchPin) {

        this->i2cHandle = i2cHandle;
        this->i2cAddress = i2cAddress; // combine default address with custom adress
        this->latchPin = latchPin;
        this->latchPort = latchPort;

        i2cDeviceAddressing = i2cDeviceCode | i2cAddress << 1;
    }

    void init() {

        // set latch pin LOW
        HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_RESET);

        uint8_t initData[2];

        // gain 2x  -> output = gain * internal reference (2048mV)

        initData[0] = 0x8F; // set output reference to internal 2048mV
        initData[1] = 0xCF; // set gain for all outputs to 2x

        // timeout 50us

        if (HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddressing, initData, 1, 50) != HAL_OK) {
            Error_Handler();
            println("I2C Dac Transmit Error");
        }
    }

    // transmit data in fastMode
    inline void fastUpdate() {
        // pData[0] |= 0b01000000; // enable FastMode

        // each half word needs to be swapped, because of reasons
        uint8_t sendOut[8];
        ((uint32_t *)sendOut)[0] = __REV16(((uint32_t *)pData)[0]);
        ((uint32_t *)sendOut)[1] = __REV16(((uint32_t *)pData)[1]);

        HAL_I2C_Master_Transmit(i2cHandle, i2cDeviceAddressing, sendOut, 8, 100);
    }

    // set the pointer to the data
    inline void setDataPointer(uint8_t *pData) { this->pData = pData; }

    I2C_HandleTypeDef *i2cHandle;
    uint8_t i2cAddress = 0;       // default address
    uint8_t i2cDeviceCode = 0xC0; // default address
    uint8_t i2cDeviceAddressing = 0;

    uint8_t *pData = nullptr;

    uint16_t latchPin;
    GPIO_TypeDef *latchPort;
};