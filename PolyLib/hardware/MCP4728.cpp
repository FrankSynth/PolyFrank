#ifdef POLYCONTROL

#include "MCP4728.hpp"

bool started = false; // global data

i2cpin i2c1Pins = {GPIOB, GPIO_PIN_7, GPIOB, GPIO_PIN_6};
// i2cpin i2c2Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};
// i2cpin i2c3Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};
// i2cpin i2c4Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};

// update all I2C Addresse ob the MCP4728 ICs
void updateI2CAddress() {

    if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
        println("I2C AddressChange - I2C Busy");
        return;
    }

    // if (HAL_I2C_GetState(&hi2cx) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // if (HAL_I2C_GetState(&hi2cx) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // if (HAL_I2C_GetState(&hi2c4x) != HAL_I2C_STATE_READY) {
    //     println("I2C AddressChange - I2C Busy");
    //     return;
    // }

    // update all MCPs on one I2C Lane

    sendI2CAddressUpdate(i2c1Pins, LDAC_1_GPIO_Port, LDAC_1_Pin);
    sendI2CAddressUpdate(i2c1Pins, LDAC_2_GPIO_Port, LDAC_2_Pin);
    sendI2CAddressUpdate(i2c1Pins, LDAC_3_GPIO_Port, LDAC_3_Pin);
    sendI2CAddressUpdate(i2c1Pins, LDAC_4_GPIO_Port, LDAC_4_Pin);

    // sendI2CAddressUpdate(i2c2Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c2Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c2Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c2Pins, GPIOx, GPIO_PIN_x);

    // sendI2CAddressUpdate(i2c3Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c3Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c3Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c3Pins, GPIOx, GPIO_PIN_x);

    // sendI2CAddressUpdate(i2c4Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c4Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c4Pins, GPIOx, GPIO_PIN_x);
    // sendI2CAddressUpdate(i2c4Pins, GPIOx, GPIO_PIN_x);
}

// send I2C Command
void sendI2CAddressUpdate(i2cpin i2cPins, GPIO_TypeDef *latchPort, uint16_t latchPin) {
    uint8_t nack = 0;

    // set I2C pins to standard gpio
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = i2cPins.pinSCL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2cPins.portSCL, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = i2cPins.pinSDA;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2cPins.portSDA, &GPIO_InitStruct);

    // use Sofware I2C to change MCP addresses

    HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_SET); // latchPin HIGH

    HAL_Delay(1);

    i2c_start_cond(i2cPins);

    nack |= i2c_write_byte(i2cPins, 0, 0, 0b11000000); // expect current i2c address is 000

    i2c_write_bit(i2cPins, 0);
    i2c_write_bit(i2cPins, 1);
    i2c_write_bit(i2cPins, 1);
    i2c_write_bit(i2cPins, 0);
    i2c_write_bit(i2cPins, 0);
    i2c_write_bit(i2cPins, 0);
    i2c_write_bit(i2cPins, 0);
    i2c_write_bit(i2cPins, 1);

    microsecondsDelay(I2CSPEED / 2);
    HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_RESET); // latchPin LOW

    nack |= i2c_read_bit(i2cPins); // ack bit

    nack |= i2c_write_byte(i2cPins, 0, 0, 0b01100001); //
    nack |= i2c_write_byte(i2cPins, 0, 0, 0b01100110); // new address 001
    nack |= i2c_write_byte(i2cPins, 0, 0, 0b01100111); // repeat new address

    i2c_stop_cond(i2cPins);

    HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_SET); // latchPin HIGH

    // reset GPIO to I2C
    GPIO_InitStruct.Pin = i2cPins.pinSCL;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(i2cPins.portSCL, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = i2cPins.pinSDA;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(i2cPins.portSDA, &GPIO_InitStruct);

    if (nack) {
        println("I2C AddressChange - NACK!");
    }
};

////// I2C Software Bit-banging //////

void I2C_delay() {
    uint32_t time = __HAL_TIM_GetCounter(&htim2);

    while (__HAL_TIM_GetCounter(&htim2) - time < I2CSPEED) {
        ;
    }
}

bool read_SCL(i2cpin pins) {
    return HAL_GPIO_ReadPin(pins.portSCL, pins.pinSCL);
} // Return current level of SCL line, 0 or 1

bool read_SDA(i2cpin pins) {
    return HAL_GPIO_ReadPin(pins.portSDA, pins.pinSDA);
} // Return current level of SDA line, 0 or 1
void set_SCL(i2cpin pins) {
    HAL_GPIO_WritePin(pins.portSCL, pins.pinSCL, GPIO_PIN_SET); // reset Latch

} // Do not drive SCL (set pin high-impedance)
void clear_SCL(i2cpin pins) {
    HAL_GPIO_WritePin(pins.portSCL, pins.pinSCL, GPIO_PIN_RESET); // reset Latch

} // Actively drive SCL signal low
void set_SDA(i2cpin pins) {
    HAL_GPIO_WritePin(pins.portSDA, pins.pinSDA, GPIO_PIN_SET); // reset Latch

} // Do not drive SDA (set pin high-impedance)
void clear_SDA(i2cpin pins) {
    HAL_GPIO_WritePin(pins.portSDA, pins.pinSDA, GPIO_PIN_RESET); // reset Latch

} // Actively drive SDA signal low
void arbitration_lost(void) {}

void i2c_start_cond(i2cpin pins) {
    if (started) {
        // if started, do a restart condition
        // set SDA to 1
        set_SDA(pins);
        I2C_delay();
        set_SCL(pins);
        while (read_SCL(pins) == 0) { // Clock stretching
                                      // You should add timeout to this loop
        }

        // Repeated start setup time, minimum 4.7us
        I2C_delay();
    }

    if (read_SDA(pins) == 0) {
        arbitration_lost();
    }

    // SCL is high, set SDA from 1 to 0.
    clear_SDA(pins);
    I2C_delay();
    clear_SCL(pins);
    started = true;
}

void i2c_stop_cond(i2cpin pins) {
    // set SDA to 0
    clear_SDA(pins);
    I2C_delay();

    set_SCL(pins);
    // Clock stretching
    while (read_SCL(pins) == 0) {
        // add timeout to this loop.
    }

    // Stop bit setup time, minimum 4us
    I2C_delay();

    // SCL is high, set SDA from 0 to 1
    set_SDA(pins);
    I2C_delay();

    if (read_SDA(pins) == 0) {
        arbitration_lost();
    }

    started = false;
}

// Write a bit to I2C bus
void i2c_write_bit(i2cpin pins, bool bit) {
    if (bit) {
        set_SDA(pins);
    }
    else {
        clear_SDA(pins);
    }

    // SDA change propagation delay
    I2C_delay();

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL(pins);

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay();

    while (read_SCL(pins) == 0) { // Clock stretching
                                  // You should add timeout to this loop
    }

    // SCL is high, now data is valid
    // If SDA is high, check that nobody else is driving SDA
    if (bit && (read_SDA(pins) == 0)) {
        arbitration_lost();
    }

    // Clear the SCL to low in preparation for next change
    clear_SCL(pins);
}

// Read a bit from I2C bus
bool i2c_read_bit(i2cpin pins) {
    bool bit;

    // Let the slave drive data
    set_SDA(pins);

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay();

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL(pins);

    while (read_SCL(pins) == 0) { // Clock stretching
                                  // You should add timeout to this loop
    }

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay();

    // SCL is high, read out bit
    bit = read_SDA(pins);

    // Set SCL low in preparation for next operation
    clear_SCL(pins);

    return bit;
}

// Write a byte to I2C bus. Return 0 if ack by the slave.
bool i2c_write_byte(i2cpin pins, bool send_start, bool send_stop, unsigned char byte) {
    unsigned bit;
    bool nack;

    if (send_start) {
        i2c_start_cond(pins);
    }

    for (bit = 0; bit < 8; ++bit) {
        i2c_write_bit(pins, (byte & 0x80) != 0);
        byte <<= 1;
    }

    nack = i2c_read_bit(pins);

    if (send_stop) {
        i2c_stop_cond(pins);
    }

    return nack;
}

// Read a byte from I2C bus
unsigned char i2c_read_byte(i2cpin pins, bool nack, bool send_stop) {
    unsigned char byte = 0;
    unsigned char bit;

    for (bit = 0; bit < 8; ++bit) {
        byte = (byte << 1) | i2c_read_bit(pins);
    }

    i2c_write_bit(pins, nack);

    if (send_stop) {
        i2c_stop_cond(pins);
    }

    return byte;
}

// helper function microsecondsDelay ---> muss noch verschoben werden

void microsecondsDelay(uint32_t delay) {
    uint32_t time = __HAL_TIM_GetCounter(&htim2);

    while (__HAL_TIM_GetCounter(&htim2) - time < delay) {
        ;
    }
}

#endif // ifdef POLYCONTROL
