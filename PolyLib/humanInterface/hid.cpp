#ifdef POLYCONTROL

#include "hid.hpp"

// PCA9555 -> Bus expander for the Encoder
PCA9555 ioExpander = PCA9555(&hi2c2, 0x00);

// create Encoder objects
rotary encoders[] = {rotary(0, 1), rotary(3, 4), rotary(6, 7), rotary(9, 10)};

#define NUMENCODERS 4

void initHID() {

    ioExpander.init();

    // register encoder

    encoders[0].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_1_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_1_CCW, &actionHandler));

    encoders[1].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_2_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_2_CCW, &actionHandler));

    encoders[2].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_3_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_3_CCW, &actionHandler));

    encoders[3].registerEventFunctions(std::bind(&actionMapping::callActionEncoder_4_CW, &actionHandler),
                                       std::bind(&actionMapping::callActionEncoder_4_CCW, &actionHandler));
}

void updateEncoder() {

    uint16_t state = ioExpander.getPinChangeRAW();
    if (state) {

        // if (state & 0x0000100100100100) { // compare input bytes with the switch mapping..... expander input
        // 2,5,8,11
        //     // sind switches

        //     // actionHandler.callActionEncoder_1_CW();
        // }
        // else { // encoder

        // encoders[0].process(state);

        for (int x = 0; x < NUMENCODERS; x++) {

            encoders[x].process(state);
        }
    }
}

#endif
