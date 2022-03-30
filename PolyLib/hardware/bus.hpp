
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
        status = "SPI\r\n";

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
        status += " STATE: ";
        if (state == BUS_READY)
            status += "READY";
        if (state == BUS_BUSY)
            status += "BUSY";
        if (state == BUS_ERROR)
            status += "ERROR";
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
            return 0;
        }
        if (state == BUS_ERROR) {
            PolyError_Handler("SPI  | bus in BUS_error state");
            return 2;
        }
        if (state == BUS_BUSY) {
            PolyError_Handler("SPI  | bus in use");
            return 1;
        }
        return 2;
    }

    void transmitReceive(uint8_t *txData, uint8_t *rxdata, uint16_t size, bool enableDMA = false) {

        if (checkState())
            return;

        rxCounter += size;
        txCounter += size;

        state = BUS_BUSY;

        if (enableDMA == true) {
            if ((hspi->hdmarx != nullptr) && (hspi->hdmatx != nullptr))
                if (HAL_SPI_TransmitReceive_DMA(hspi, txData, rxdata, size) != HAL_OK)
                    state = BUS_ERROR;
        }
        else if (enableDMA == false) {
            if (HAL_SPI_TransmitReceive(hspi, txData, rxdata, size, timeout) != HAL_OK) {

                state = BUS_ERROR;
            }
            else {
                state = BUS_READY;
            }
        }
    };
    void transmit(uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (checkState())
            return;

        txCounter += size;

        state = BUS_BUSY;

        if (enableDMA == true) {
            if (hspi->hdmarx != nullptr)

                if (HAL_SPI_Transmit_DMA(hspi, data, size) != HAL_OK)
                    state = BUS_ERROR;
        }
        else if (enableDMA == false) {
            if (HAL_SPI_Transmit(hspi, data, size, timeout) != HAL_OK) {
                state = BUS_ERROR;
            }
            else {
                state = BUS_READY;
            }
        }
    };
    void receive(uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (checkState())
            return;

        rxCounter += size;

        state = BUS_BUSY;

        if (enableDMA == true) {
            if (hspi->hdmatx != nullptr)
                if (HAL_SPI_Receive_DMA(hspi, data, size) != HAL_OK)
                    state = BUS_ERROR;
        }
        else if (enableDMA == false) {
            if (HAL_SPI_Receive(hspi, data, size, timeout) != HAL_OK) {
                state = BUS_ERROR;
            }
            else {
                state = BUS_READY;
            }
        }
    };

    SPI_HandleTypeDef *hspi;

    uint32_t timeout = 100;
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
        status = "I2C\r\n";

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
        status += " STATE: ";
        if (state == BUS_READY)
            status += "READY";
        if (state == BUS_BUSY)
            status += "BUSY";
        if (state == BUS_ERROR)
            status += "ERROR";
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
            return 0;
        }
        if (state == BUS_ERROR) {
            PolyError_Handler("I2C  | bus in BUS_error state");
            return 2;
        }
        if (state == BUS_BUSY) {
            PolyError_Handler("I2C  | bus in use");
            return 1;
        }
        return 2;
    }
    void transmit(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {

        if (checkState())
            return;

        txCounter += size;

        state = BUS_BUSY;

        if (enableDMA == true) {
            if (hi2c->hdmarx != nullptr)
                if (HAL_I2C_Master_Transmit_DMA(hi2c, address, data, size) != HAL_OK)
                    state = BUS_ERROR;
        }
        else if (enableDMA == false) {
            if (HAL_I2C_Master_Transmit(hi2c, address, data, size, timeout) != HAL_OK) {
                state = BUS_ERROR;
            }
            else {
                state = BUS_READY;
            }
        }
    };
    void receive(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        if (checkState())
            return;

        rxCounter += size;

        state = BUS_BUSY;

        if (enableDMA == true) {
            if (hi2c->hdmatx != nullptr)
                if (HAL_I2C_Master_Receive_DMA(hi2c, address, data, size) != HAL_OK)
                    state = BUS_ERROR;
        }
        else if (enableDMA == false) {
            if (HAL_I2C_Master_Receive(hi2c, address, data, size, timeout) != HAL_OK) {
                state = BUS_ERROR;
            }
            else {
                state = BUS_READY;
            }
        }
    };

    I2C_HandleTypeDef *hi2c;

    uint32_t timeout = 100;
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
    };

    void connectToMultiplexer(PCA9548 *busMultiplexer, uint8_t virtualBusAddress) {
        this->busMultiplexer = busMultiplexer;
        this->virtualBusAddress = virtualBusAddress;
    }
    void connectToBus(i2cBus *busi2c) { this->busi2c = busi2c; }

    std::string *report() {

        status.clear();

        status = "virtual I2C Bus   :";
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

    void transmit(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        VirtualTxCounter += size;

        busMultiplexer->switchVirtualBus(virtualBusAddress);
        busi2c->transmit(address, data, size, enableDMA);
    };
    void receive(uint16_t address, uint8_t *data, uint16_t size, bool enableDMA = false) {
        VirtualRxCounter += size;
        busi2c->receive(address, data, size, enableDMA);
    };

    i2cBus *busi2c;

    uint32_t VirtualRxCounter = 0;
    uint32_t VirtualTxCounter = 0;

    uint8_t virtualBusAddress = 0;
    PCA9548 *busMultiplexer = nullptr;
};
