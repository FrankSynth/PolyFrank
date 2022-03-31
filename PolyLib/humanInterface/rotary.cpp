#ifdef POLYCONTROL

/* Rotary encoder handler for arduino.
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 */

// Use the full-step state table (emits a code at 00 only)

#include "rotary.hpp"

float ROTARYENCODERACCELERATION = 0; // store the current Encoder Acceleration for calculation changes

const unsigned char ttable[7][4] = {
    // R_START
    {R_START, R_CW_BEGIN, R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT, R_START, R_CW_FINAL, R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT, R_CW_BEGIN, R_START, R_START},
    // R_CW_NEXT
    {R_CW_NEXT, R_CW_BEGIN, R_CW_FINAL, R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START, R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START, R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

void rotary::process(uint16_t pinState) {

    uint8_t pin1State = 0;
    uint8_t pin2State = 0;

    // println(pinState);

    if (pinState & (1 << pin1)) {
        pin1State = 1;
    }

    if (pinState & (1 << pin2)) {
        pin2State = 1;
    }

    //  println(pin1State);
    //  println(pin2State);

    // Grab state of input pins.
    unsigned char pinstate = (pin2State << 1) | pin1State;
    // Determine new state from the pins and state table.
    state = ttable[state & 0xf][pinstate];
    // Return emit bits, ie the generated event.
    if ((state & 0x30) == DIR_CW) {

        acellaration();

        if (functionCW != nullptr) {
            functionCW();
        }
        lastStepTime = millis();
    }
    else if ((state & 0x30) == DIR_CCW) {
        acellaration();

        if (functionCCW != nullptr) {
            functionCCW();
        }

        lastStepTime = millis();
    }
}

void rotary::acellaration() {
    ROTARYENCODERACCELERATION = std::clamp(1.0f / (float)(millis() - lastStepTime), 0.001f, 1.0f);
}

#endif