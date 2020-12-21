
#ifdef POLYCONTROL

#include "guiActionHandler.hpp"

actionMapping actionHandler;

void actionMapping::registerActionHeader(actionHandle handle1, actionHandle handle2, actionHandle handle3,
                                         actionHandle handle4) {
    buttonHeader_1 = handle1;
    buttonHeader_2 = handle2;
    buttonHeader_3 = handle3;
    buttonHeader_4 = handle4;
}

void actionMapping::registerActionLeft(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    buttonLeft_1 = handle1;
    buttonLeft_2 = handle2;
    buttonLeft_3 = handle3;
}

void actionMapping::registerActionRight(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    buttonRight_1 = handle1;
    buttonRight_2 = handle2;
    buttonRight_3 = handle3;
}

void actionMapping::registerActionEncoder1(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder1_CW = handle1;
    encoder1_CCW = handle2;
    encoder1_Push = handle3;
}
void actionMapping::registerActionEncoder2(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder2_CW = handle1;
    encoder2_CCW = handle2;
    encoder2_Push = handle3;
}
void actionMapping::registerActionEncoder3(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder3_CW = handle1;
    encoder3_CCW = handle2;
    encoder3_Push = handle3;
}
void actionMapping::registerActionEncoder4(actionHandle handle1, actionHandle handle2, actionHandle handle3) {
    encoder4_CW = handle1;
    encoder4_CCW = handle2;
    encoder4_Push = handle3;
}

void actionMapping::callActionHeader1() {
    if (buttonHeader_1.functionPointer != nullptr)
        buttonHeader_1.functionPointer();
};

void actionMapping::callActionHeader2() {
    if (buttonHeader_2.functionPointer != nullptr)
        buttonHeader_2.functionPointer();
};

void actionMapping::callActionHeader3() {
    if (buttonHeader_3.functionPointer != nullptr)
        buttonHeader_3.functionPointer();
};

void actionMapping::callActionHeader4() {
    if (buttonHeader_4.functionPointer != nullptr)
        buttonHeader_4.functionPointer();
};

void actionMapping::callActionLeft1() {
    if (buttonLeft_1.functionPointer != nullptr)
        buttonLeft_1.functionPointer();
};

void actionMapping::callActionLeft2() {
    if (buttonLeft_2.functionPointer != nullptr)
        buttonLeft_2.functionPointer();
};

void actionMapping::callActionLeft3() {
    if (buttonLeft_3.functionPointer != nullptr)
        buttonLeft_3.functionPointer();
};

void actionMapping::callActionRight1() {
    if (buttonRight_1.functionPointer != nullptr)
        buttonRight_1.functionPointer();
};

void actionMapping::callActionRight2() {
    if (buttonRight_2.functionPointer != nullptr)
        buttonRight_2.functionPointer();
};

void actionMapping::callActionRight3() {
    if (buttonRight_3.functionPointer != nullptr)
        buttonRight_3.functionPointer();
};

void actionMapping::callActionEncoder_1_CW() {
    if (encoder1_CW.functionPointer != nullptr)
        encoder1_CW.functionPointer();
};

void actionMapping::callActionEncoder_1_CCW() {
    if (encoder1_CCW.functionPointer != nullptr)
        encoder1_CCW.functionPointer();
};

void actionMapping::callActionEncoder_1_Push() {
    if (encoder1_Push.functionPointer != nullptr)
        encoder1_Push.functionPointer();
};

void actionMapping::callActionEncoder_2_CW() {
    if (encoder2_CW.functionPointer != nullptr)
        encoder2_CW.functionPointer();
};

void actionMapping::callActionEncoder_2_CCW() {
    if (encoder2_CCW.functionPointer != nullptr)
        encoder2_CCW.functionPointer();
};

void actionMapping::callActionEncoder_2_Push() {
    if (encoder2_Push.functionPointer != nullptr)
        encoder2_Push.functionPointer();
};

void actionMapping::callActionEncoder_3_CW() {
    if (encoder3_CW.functionPointer != nullptr)
        encoder3_CW.functionPointer();
};

void actionMapping::callActionEncoder_3_CCW() {
    if (encoder3_CCW.functionPointer != nullptr)
        encoder3_CCW.functionPointer();
};

void actionMapping::callActionEncoder_3_Push() {
    if (encoder3_Push.functionPointer != nullptr)
        encoder3_Push.functionPointer();
};

void actionMapping::callActionEncoder_4_CW() {
    if (encoder4_CW.functionPointer != nullptr)
        encoder4_CW.functionPointer();
};

void actionMapping::callActionEncoder_4_CCW() {
    if (encoder4_CCW.functionPointer != nullptr)
        encoder4_CCW.functionPointer();
};

void actionMapping::callActionEncoder_4_Push() {
    if (encoder4_Push.functionPointer != nullptr)
        encoder4_Push.functionPointer();
};

#endif