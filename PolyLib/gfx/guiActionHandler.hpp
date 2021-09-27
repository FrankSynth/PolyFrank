#pragma once

#include <functional>
#include <string>

typedef struct {
    std::function<void()> functionPointer = nullptr;
    std::string name = "";
} actionHandle;

typedef enum { RELEASED, PRESSED } BUTTONSTATE;

typedef struct {
    actionHandle handle;
    BUTTONSTATE state = RELEASED;
} ButtonActionHandle; // for buttons with press state

class actionMapping {
  public:
    // Register Touch
    void registerActionHeader(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                              actionHandle handle3 = {nullptr, ""}, actionHandle handle4 = {nullptr, ""});

    void registerActionLeft(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                            actionHandle handle3 = {nullptr, ""});

    void registerActionRight(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                             actionHandle handle3 = {nullptr, ""});

    void registerActionRight2(actionHandle handle2);

    void registerActionRight3(actionHandle handle3);

    // Register Rotary Encoder

    void registerActionEncoder1(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder2(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder3(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder4(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder5(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder6(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void callActionHeader1();

    void callActionHeader2();

    void callActionHeader3();

    void callActionHeader4();

    void callActionLeft1();

    void callActionLeft2();

    void callActionLeft3();

    void callActionRight1();

    void callActionRight2();

    void callActionRight3();

    void callActionEncoder_1_CW();
    void callActionEncoder_1_CCW();
    void callActionEncoder_1_Push();

    void callActionEncoder_2_CW();
    void callActionEncoder_2_CCW();
    void callActionEncoder_2_Push();

    void callActionEncoder_3_CW();
    void callActionEncoder_3_CCW();
    void callActionEncoder_3_Push();

    void callActionEncoder_4_CW();
    void callActionEncoder_4_CCW();
    void callActionEncoder_4_Push();

    void callActionEncoder_5_CW();
    void callActionEncoder_5_CCW();
    void callActionEncoder_5_Push();

    void callActionEncoder_6_CW();
    void callActionEncoder_6_CCW();
    void callActionEncoder_6_Push();

    actionHandle buttonHeader_1;
    actionHandle buttonHeader_2;
    actionHandle buttonHeader_3;
    actionHandle buttonHeader_4;
    actionHandle buttonHeader_5;
    actionHandle buttonHeader_6;

    ButtonActionHandle buttonLeft_1;
    ButtonActionHandle buttonLeft_2;
    ButtonActionHandle buttonLeft_3;
    ButtonActionHandle buttonRight_1;
    ButtonActionHandle buttonRight_2;
    ButtonActionHandle buttonRight_3;

    actionHandle encoder1_CW;
    actionHandle encoder1_CCW;
    actionHandle encoder1_Push;

    actionHandle encoder2_CW;
    actionHandle encoder2_CCW;
    actionHandle encoder2_Push;

    actionHandle encoder3_CW;
    actionHandle encoder3_CCW;
    actionHandle encoder3_Push;

    actionHandle encoder4_CW;
    actionHandle encoder4_CCW;
    actionHandle encoder4_Push;

    actionHandle encoder5_CW;
    actionHandle encoder5_CCW;
    actionHandle encoder5_Push;

    actionHandle encoder6_CW;
    actionHandle encoder6_CCW;
    actionHandle encoder6_Push;
};

extern actionMapping actionHandler;