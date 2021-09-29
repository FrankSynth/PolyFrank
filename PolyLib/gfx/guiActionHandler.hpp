#pragma once

#include <functional>
#include <string>

#define NUMBUTTONSIDE 3
#define NUMBUTTONTOP 4
#define NUMBUTTONBOTTOM 4
#define NUMENCODER 5

typedef struct {
    std::function<void()> functionPointer = nullptr;
    std::string name = "";
} actionHandle;

typedef enum { RELEASED, PRESSED } BUTTONSTATE;

typedef struct {
    actionHandle handle;
    BUTTONSTATE state = RELEASED;
    int32_t *data = nullptr;
    uint8_t unlock = 0;

} ButtonActionHandle; // for buttons with press state
typedef struct {
    actionHandle handle_CW;
    actionHandle handle_CCW;
    actionHandle handle_PUSH;
} EncoderActionHandle; // for buttons with press state

class actionMapping {
  public:
    // Register Touch
    void registerActionHeader(uint8_t index, actionHandle handle = {nullptr, ""});

    void registerActionFooter(uint8_t index, actionHandle handle = {nullptr, ""});

    void registerActionEncoder(uint8_t index, actionHandle handleCW = {nullptr, ""},
                               actionHandle handleCCW = {nullptr, ""}, actionHandle handlePUSH = {nullptr, ""});

    void registerActionLeftData(uint8_t index, actionHandle handle = {nullptr, ""}, int32_t *data = nullptr,
                                uint8_t lock = 1);
    void registerActionRightData(uint8_t index, actionHandle handle = {nullptr, ""}, int32_t *data = nullptr,
                                 uint8_t lock = 1);

    void registerActionLeft(uint8_t index, actionHandle handle = {nullptr, ""}, uint8_t lock = 1);
    void registerActionRight(uint8_t index, actionHandle handle = {nullptr, ""}, uint8_t lock = 1);

    void callActionHeader(uint8_t index);
    void callActionLeft(uint8_t index);
    void callActionRight(uint8_t index);

    void callActionEncoder_CW(uint8_t index);
    void callActionEncoder_CCW(uint8_t index);
    void callActionEncoder_Push(uint8_t index);

    void lock(uint8_t unlock) { unlocked = unlock; }

    actionHandle buttonFooter[NUMBUTTONBOTTOM];
    actionHandle buttonHeader[NUMBUTTONTOP];

    ButtonActionHandle buttonLeft[NUMBUTTONSIDE];
    ButtonActionHandle buttonRight[NUMBUTTONSIDE];

    EncoderActionHandle encoder[NUMENCODER];

    uint8_t unlocked = 0;
};

extern actionMapping actionHandler;