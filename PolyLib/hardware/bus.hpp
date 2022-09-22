
#pragma once

#include "hardware/busInterface.hpp"
#include "hardware/driver.hpp"
#include "i2c.h"
#include "spi.h"

#include "debughelper/debughelper.hpp"

// SPI bus class
class spiBus : public busInterface {
  public:
    spiBus() { type = SPI; };
    void connectToInterface(SPI_HandleTypeDef *hspi) {
        this->hspi = hspi;
        state = BUS_READY;
    }

    std::string *report() {
        status = "\nSPI\r\n";

        // DMA
        if ((hspi->hdmarx != nullptr) | (hspi->hdmatx != nullptr)) {
            status += " DMA: ";
            if (hspi->hdmarx != nullptr)
                status += "RX ";
            if (hspi->hdmatx != nullptr)
                status += "TX ";
            status += "\r\n";
        }
        // STATE
        status += " State: ";
        if (state == BUS_READY)
            status += "ready";
        else if (state == BUS_BUSY)
            status += "busy";
        else if (state == BUS_SEND)
            status += "send";
        else if (state == BUS_RECEIVE)
            status += "receive";
        else if (state == BUS_SENDRECEIVE)
            status += "send receive";
        else if (state == BUS_ERROR)
            status += "error";
        status += "\r\n";

        // DATA
        status += " Packets: ";
        status += " RX " + std::to_string(rxCounter);
        status += " TX " + std::to_string(txCounter);

        status += "\r\n";

        return &status;
    }

    uint8_t checkState() {
        // if (state == BUS_READY) {
        // return 0;
        // }
        if (state == BUS_ERROR) {
            PolyError_Handler("SPI  | bus in BUS_error state");
            // return 2;
        }
        // if (state == BUS_BUSY) {
        // PolyError_Handler("SPI  | bus in use");
        // return 1;
        // }
        return state;
    }

    busState transmitReceive(uint8_t *txData, uint8_t *rxdata, uint16_t size, bool enableDMA = false) {

        if (state == BUS_ERROR)
            return state;

        rxCounter += size;
        txCounter += size;

        state = BUS_SENDRECEIVE;

        if (enableDMA) {
            if ((hspi->hdmarx != nullptr) && (hspi->hdmatx != nullptr)) {

                HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive_DMA(hspi, txData, rxdata, size);
                if (ret == HAL_ERROR || ret == HAL_TIMEOUT) {
                    state = BUS_ERROR;
                    return state;
                }
                // println("dma started");
            }
            else {
                return BUS_ERROR;
            }
        }
        else {
            __disable_irq();
            HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive(hspi, txData, rxdata, size, timeout);
            __enable_irq();
            if (ret == HAL_ERROR) {
                state = BUS_ERROR;
                return state;
            }
            else if (ret == HAL_BUSY) {
                state = BUS_BUSY;
                return state;
            }
            else {
                state = BUS_READY;
            }
        }
        return BUS_OK;
    }
    busState transmit(uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (state == BUS_ERROR)
            return state;

        txCounter += size;

        state = BUS_SEND;

        if (enableDMA) {
            if (hspi->hdmatx != nullptr) {

                HAL_StatusTypeDef ret = HAL_SPI_Transmit_DMA(hspi, data, size);
                if (ret == HAL_ERROR || ret == HAL_TIMEOUT) {
                    state = BUS_ERROR;
                    return state;
                }
            }
            else {
                return BUS_ERROR;
            }
        }
        else {
            __disable_irq();
            HAL_StatusTypeDef ret = HAL_SPI_Transmit(hspi, data, size, timeout);
            __enable_irq();
            if (ret == HAL_ERROR) {
                state = BUS_ERROR;
                return state;
            }
            else if (ret == HAL_BUSY) {
                state = BUS_BUSY;
                return state;
            }
            else {
                state = BUS_READY;
            }
        }
        return BUS_OK;
    }
    busState receive(uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (state == BUS_ERROR)
            return state;

        rxCounter += size;

        state = BUS_RECEIVE;

        if (enableDMA) {
            if (hspi->hdmarx != nullptr) {
                HAL_StatusTypeDef ret = HAL_SPI_Receive_DMA(hspi, data, size);
                if (ret == HAL_ERROR || ret == HAL_TIMEOUT) {
                    state = BUS_ERROR;
                    return state;
                }
            }
            else {
                return BUS_ERROR;
            }
        }
        else {
            __disable_irq();
            HAL_StatusTypeDef ret = HAL_SPI_Receive(hspi, data, size, timeout);
            __enable_irq();
            if (ret == HAL_ERROR) {
                state = BUS_ERROR;
                return state;
            }
            else if (ret == HAL_BUSY) {
                state = BUS_BUSY;
                return state;
            }
            else {
                state = BUS_READY;
            }
        }
        return BUS_OK;
    }
    busState stopDMA() {
        if (state == BUS_RECEIVE || state == BUS_SEND || state == BUS_SENDRECEIVE)
            if (HAL_SPI_Abort(hspi) == HAL_OK) {
                state = BUS_READY;
                return BUS_OK;
            }
        if (state == BUS_READY)
            return BUS_OK;

        state = BUS_ERROR;
        return state;
    }

    SPI_HandleTypeDef *hspi;

    uint32_t timeout = 1000;
};

// I2C bus class
class i2cBus : public busInterface {
  public:
    i2cBus() { type = I2C; };

    void connectToInterface(I2C_HandleTypeDef *hi2c) {
        this->hi2c = hi2c;
        state = BUS_READY;
    }

    std::string *report() {
        status = "\nI2C\r\n";

        // DMA
        if ((hi2c->hdmarx != nullptr) | (hi2c->hdmatx != nullptr)) {
            status += " DMA: ";
            if (hi2c->hdmarx != nullptr)
                status += "RX ";
            if (hi2c->hdmatx != nullptr)
                status += "TX ";
            status += "\r\n";
        }

        // STATE
        status += " State: ";
        if (state == BUS_READY)
            status += "ready";
        if (state == BUS_BUSY)
            status += "busy";
        if (state == BUS_ERROR)
            status += "error";
        status += "\r\n";

        // DATA
        status += " Packets: ";
        status += " RX " + std::to_string(rxCounter);
        status += " TX " + std::to_string(txCounter);

        status += "\r\n";

        return &status;
    }

    uint8_t checkState() {

        if (state == BUS_READY) {
            // return 0;
        }
        if (state == BUS_ERROR) {
            PolyError_Handler("I2C  | bus in BUS_error state");
            // return 2;
        }
        if (state == BUS_BUSY) {
            // PolyError_Handler("I2C  | bus in use");
            // return 1;
        }
        return state;
    }
    busState transmit(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (state == BUS_ERROR)
            return state;

        txCounter += size;

        state = BUS_SEND;

        if (enableDMA) {
            if (hi2c->hdmatx != nullptr) {

                HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit_DMA(hi2c, address, data, size);
                if (ret == HAL_ERROR || ret == HAL_TIMEOUT) {
                    state = BUS_ERROR;
                    return state;
                }
            }
            else {
                return BUS_ERROR;
            }
        }
        else {
            __disable_irq();
            HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c, address, data, size, timeout);
            __enable_irq();
            if (ret == HAL_ERROR) {
                state = BUS_ERROR;
                return state;
            }
            else if (ret == HAL_BUSY) {
                state = BUS_BUSY;
                return state;
            }
            else {
                state = BUS_READY;
            }
        }
        return BUS_OK;
    }
    busState receive(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        if (state == BUS_ERROR)
            return state;

        rxCounter += size;

        state = BUS_RECEIVE;

        if (enableDMA) {
            if (hi2c->hdmarx != nullptr) {
                HAL_StatusTypeDef ret = HAL_I2C_Master_Receive_DMA(hi2c, address, data, size);
                if (ret == HAL_ERROR || ret == HAL_TIMEOUT) {
                    state = BUS_ERROR;
                    return state;
                }
            }
            else {
                return BUS_ERROR;
            }
        }
        else {
            __disable_irq();
            HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(hi2c, address, data, size, timeout);
            __enable_irq();
            if (ret == HAL_ERROR) {
                state = BUS_ERROR;
                return state;
            }
            else if (ret == HAL_BUSY) {
                state = BUS_BUSY;
                return state;
            }
            else {
                state = BUS_READY;
            }
        }
        return BUS_OK;
    }

    I2C_HandleTypeDef *hi2c;

    uint32_t timeout = 1000;
};

class PCA9548 : public baseDevice {
  public:
    void configurate(i2cBus *busInterface, uint8_t i2cDeviceAddress) {
        this->busInterface = busInterface;
        this->i2cDeviceAddress = i2cDeviceCode | i2cDeviceAddress << 1;

        state = DEVICE_READY;
        deviceName = "PCA9548";
    }

    void switchVirtualBus(uint8_t VirtualBus) {

        if (activeVirtualBus != (1 << VirtualBus)) {
            activeVirtualBus = (1 << VirtualBus);
            busInterface->transmit(i2cDeviceAddress, &activeVirtualBus, 1);
        }
    }

    uint8_t activeVirtualBus = 0x00;

    i2cBus *busInterface;

    uint8_t i2cDeviceCode = 0xE0; // default Code
    uint8_t i2cDeviceAddress = 0;
};

// I2C bus class
class i2cVirtualBus : public busInterface {
  public:
    i2cVirtualBus() {
        type = VIRTUALI2C;
        state = BUS_READY;
    }

    void connectToMultiplexer(PCA9548 *busMultiplexer, uint8_t virtualBusAddress) {
        this->busMultiplexer = busMultiplexer;
        this->virtualBusAddress = virtualBusAddress;
    }
    void connectToBus(i2cBus *busi2c) { this->busi2c = busi2c; }

    std::string *report() {

        status.clear();

        status = "\nVirtual I2C Bus | ID: ";
        status += std::to_string(virtualBusAddress);
        status += "\r\n";
        // DATA
        status += " Packets: ";
        status += " RX " + std::to_string(VirtualRxCounter);
        status += " TX " + std::to_string(VirtualTxCounter);

        status += "\r\n";

        // return &status;

        return &status;
    }

    busState transmit(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        VirtualTxCounter += size;

        busMultiplexer->switchVirtualBus(virtualBusAddress);
        return busi2c->transmit(address, data, size, enableDMA);
    }
    busState receive(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        VirtualRxCounter += size;
        return busi2c->receive(address, data, size, enableDMA);
    }

    i2cBus *busi2c;

    uint32_t VirtualRxCounter = 0;
    uint32_t VirtualTxCounter = 0;

    uint8_t virtualBusAddress = 0;
    PCA9548 *busMultiplexer = nullptr;
};
