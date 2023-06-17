#pragma once

#include <functional>
#include <string>

#define NUMBUTTONSIDE 3
#define NUMBUTTONTOP 4
#define NUMBUTTONBOTTOM 4
#define NUMENCODER 6

typedef struct {
    std::function<void()> functionPointer = nullptr;
    std::string name = "";
    uint32_t pendingCalls = 0;

} actionHandle;

typedef enum { RELEASED, PRESSED } BUTTONSTATE;

typedef struct {
    actionHandle handle;
    BUTTONSTATE state = RELEASED;
    int32_t *data = nullptr;
    uint32_t unlock = 0;

} ButtonActionHandle; // for buttons with press state
typedef struct {
    actionHandle handle_CW;
    actionHandle handle_CCW;
    actionHandle handle_PUSH;

} EncoderActionHandle; // for buttons with press state

class actionMapping {
  public:
    // Register Touch
    void registerActionHeader(uint32_t index, actionHandle handle = {nullptr, ""});

    void registerActionFooter(uint32_t index, actionHandle handle = {nullptr, ""});

    void registerActionEncoder(uint32_t index, actionHandle handleCW = {nullptr, ""},
                               actionHandle handleCCW = {nullptr, ""}, actionHandle handlePUSH = {nullptr, ""});

    void registerActionLeftData(uint32_t index, actionHandle handle = {nullptr, ""}, int32_t *data = nullptr,
                                uint32_t lock = 0);
    void registerActionRightData(uint32_t index, actionHandle handle = {nullptr, ""}, int32_t *data = nullptr,
                                 uint32_t lock = 0);

    void registerActionLeft(uint32_t index, actionHandle handle = {nullptr, ""}, uint32_t lock = 0);
    void registerActionRight(uint32_t index, actionHandle handle = {nullptr, ""}, uint32_t lock = 0);

    void callActionFooter(uint32_t index);
    void callActionHeader(uint32_t index);
    void callActionLeft(uint32_t index);
    void callActionRight(uint32_t index);

    void callActionEncoder_CW(uint32_t index);
    void callActionEncoder_CCW(uint32_t index);
    void callActionEncoder_Push(uint32_t index);

    void ActionHandlerServiceRoutine();

    void clear();

    actionHandle buttonFooter[NUMBUTTONBOTTOM];
    actionHandle buttonHeader[NUMBUTTONTOP];

    ButtonActionHandle buttonLeft[NUMBUTTONSIDE];
    ButtonActionHandle buttonRight[NUMBUTTONSIDE];

    EncoderActionHandle encoder[NUMENCODER];
};

extern actionMapping actionHandler;