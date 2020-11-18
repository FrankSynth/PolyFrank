#ifdef POLYCONTROL
#pragma once

#include "gfx/gui.hpp"
#include <functional>
#define DIR_NONE 0x0
#define DIR_CW 0x10
#define DIR_CCW 0x20

#define R_START 0x0
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

class rotary {
  public:
    rotary(char pin1, char pin2) {
        this->pin1 = pin1;
        this->pin2 = pin2;
    }

    void registerEventFunctions(std::function<void()> functionCW, std::function<void()> functionCCW) {
        this->functionCW = functionCW;
        this->functionCCW = functionCCW;
    }
    void process(uint16_t pinState);

  private:
    std::function<void()> functionCW = nullptr;
    std::function<void()> functionCCW = nullptr;

    unsigned char state = R_START;
    unsigned char pin1;
    unsigned char pin2;
};

#endif