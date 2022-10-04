#include "MAX11128.hpp"
#include "datacore/datalocation.hpp"

void MAX11128::fetchNewData() {

    while (busInterface->state == BUS_SENDRECEIVE) { // wait for other adc free bus
    }
    HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    // reset sampleCommand.. SPI TransmitReceive will corrupt the transmit buffer!
    static uint32_t zero = 0;
    fastMemset(&zero, command, nChannels - 1);

    command[nChannels - 1] = standardSampleCommand;
    state = DEVICE_BUSY;
    busInterface->transmitReceive((uint8_t *)command, (uint8_t *)adcData, nChannels, true);
    // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
    // println("transmit Receive");
}