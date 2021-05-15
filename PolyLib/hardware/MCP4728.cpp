#ifdef POLYRENDER

#include "MCP4728.hpp"

bool started = false; // global data

i2cpin i2c1Pins = {GPIOB, GPIO_PIN_7, GPIOB, GPIO_PIN_6};
// i2cpin i2c2Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};
// i2cpin i2c3Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};
// i2cpin i2c4Pins = {GPIOx, GPIO_PIN_x, GPIOx, GPIO_PIN_x};

// send I2C Command
void sendI2CAddressUpdate(i2cpin i2cPins, GPIO_TypeDef *latchPort, uint16_t latchPin, uint8_t fromAddress,
                          uint8_t toAddress) {
    uint8_t nack = 0;

    HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_SET); // latchPin LOW

    // set I2C pins to standard gpio
    HAL_GPIO_WritePin(i2cPins.portSCL, i2cPins.pinSCL, GPIO_PIN_SET); // reset Latch
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = i2cPins.pinSCL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2cPins.portSCL, &GPIO_InitStruct);

    HAL_GPIO_WritePin(i2cPins.portSDA, i2cPins.pinSDA, GPIO_PIN_SET); // reset Latch
    GPIO_InitStruct.Pin = i2cPins.pinSDA;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2cPins.portSDA, &GPIO_InitStruct);

    // use Sofware I2C to change MCP addresses

    uint8_t command = 0b01100001 | (fromAddress << 2);

    i2c_start_cond(i2cPins);

    nack |= i2c_write_byte(i2cPins, 0, 0, 0b11000000 | (fromAddress << 1)); // write device address to Bus
    microsecondsDelay(2);
    i2c_write_bit(i2cPins, command & (0x01 << 7));
    i2c_write_bit(i2cPins, command & (0x01 << 6));
    i2c_write_bit(i2cPins, command & (0x01 << 5));
    i2c_write_bit(i2cPins, command & (0x01 << 4));
    i2c_write_bit(i2cPins, command & (0x01 << 3));
    i2c_write_bit(i2cPins, command & (0x01 << 2));
    i2c_write_bit(i2cPins, command & (0x01 << 1));
    i2c_write_bit(i2cPins, command & 0x01);

    nack |= i2c_read_bit_andLatch(i2cPins, latchPort, latchPin); // ack bit

    microsecondsDelay(2);
    nack |= i2c_write_byte(i2cPins, 0, 0, 0b01100010 | (toAddress << 2)); // new address 001
    microsecondsDelay(2);
    nack |= i2c_write_byte(i2cPins, 0, 0, 0b01100011 | (toAddress << 2)); // repeat new address

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
        println("I2C AddressChange - NACK!", "-> Address change from ", fromAddress, " to ", toAddress, " failed ");
    }
    else {
        println("I2C Address changed", "-> from ", fromAddress, " to ", toAddress);
    }

    HAL_Delay(100);
};

////// I2C Software Bit-banging //////

void I2C_delay() {
    uint32_t time = __HAL_TIM_GetCounter(&htim2);

    while (__HAL_TIM_GetCounter(&htim2) - time < I2CSPEED) {
        ;
    }
}

void I2C_HalfDelay() {
    uint32_t time = __HAL_TIM_GetCounter(&htim2);

    while (__HAL_TIM_GetCounter(&htim2) - time < (I2CSPEED / 2)) {
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
    I2C_HalfDelay();
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

// Read a bit from I2C bus
bool i2c_read_bit_andLatch(i2cpin pins, GPIO_TypeDef *latchPort, uint16_t latchPin) {
    bool bit;

    // Let the slave drive data
    set_SDA(pins);

    // latch pin for MCP address change

    HAL_GPIO_WritePin(latchPort, latchPin, GPIO_PIN_RESET); // latchPin LOW
    I2C_HalfDelay();
    I2C_HalfDelay();

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

#endif // ifdef POLYCONTROL
