#pragma once

#include <functional>
#include <string>

typedef struct {
    std::function<void()> functionPointer = nullptr;
    std::string name = "";
} actionHandle;

class actionMapping {
  public:
    void registerActionHeader(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                              actionHandle handle3 = {nullptr, ""}, actionHandle handle4 = {nullptr, ""});

    void registerActionLeft(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                            actionHandle handle3 = {nullptr, ""});

    void registerActionRight(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                             actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder1(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});
    void registerActionEncoder2(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder3(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
                                actionHandle handle3 = {nullptr, ""});

    void registerActionEncoder4(actionHandle handle1 = {nullptr, ""}, actionHandle handle2 = {nullptr, ""},
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

    actionHandle buttonHeader_1;
    actionHandle buttonHeader_2;
    actionHandle buttonHeader_3;
    actionHandle buttonHeader_4;

    actionHandle buttonLeft_1;
    actionHandle buttonLeft_2;
    actionHandle buttonLeft_3;

    actionHandle buttonRight_1;
    actionHandle buttonRight_2;
    actionHandle buttonRight_3;

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
};

extern actionMapping actionHandler;