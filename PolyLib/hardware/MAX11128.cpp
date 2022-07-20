#include "MAX11128.hpp"
#include "datacore/datalocation.hpp"

RAM2_DMA ALIGN_32BYTES(volatile uint32_t maxBufferIn[16]);
RAM2_DMA ALIGN_32BYTES(volatile uint32_t maxBufferOut[16]);

void MAX11128::fetchNewData() {

    // reset sampleCommand.. SPI TransmitReceive will corrupt the transmit buffer!
    static uint32_t zero = 0;
    fastMemset(&zero, command, nChannels - 1);

    command[nChannels - 1] = standardSampleCommand;

    // fast_copy_f32(command, (uint32_t *)maxBufferOut, nChannels);

    // // receive new samples and send sample command
    // // for (uint32_t i = 0; i < nChannels; i++) {
    // // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);

    // if (busInterface->transmitReceive((uint8_t *)maxBufferOut, (uint8_t *)maxBufferIn, nChannels, true) == BUS_OK) {
    //     while (busInterface->state != BUS_READY) {
    //         // microsecondsDelay(1);
    //     }
    //     fast_copy_f32((uint32_t *)maxBufferIn, adcData, nChannels);
    // }

    busInterface->transmitReceive((uint8_t *)command, (uint8_t *)adcData, nChannels, false);

    // HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
    // }
}