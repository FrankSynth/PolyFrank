
#ifdef POLYCONTROL

#include "guiActionHandler.hpp"

actionMapping actionHandler;

void actionMapping::registerActionHeader(uint8_t index, actionHandle handle) {
    buttonHeader[index] = handle;
}

void actionMapping::registerActionFooter(uint8_t index, actionHandle handle) {
    buttonFooter[index] = handle;
}

void actionMapping::registerActionLeftData(uint8_t index, actionHandle handle, int32_t *data, uint8_t lock) {
    buttonLeft[index].handle = handle;
    buttonLeft[index].data = data;
    buttonLeft[index].unlock = !lock;
}

void actionMapping::registerActionRightData(uint8_t index, actionHandle handle, int32_t *data, uint8_t lock) {
    buttonRight[index].handle = handle;
    buttonRight[index].data = data;
    buttonRight[index].unlock = !lock;
}

void actionMapping::registerActionLeft(uint8_t index, actionHandle handle, uint8_t lock) {
    buttonLeft[index].handle = handle;
    buttonLeft[index].data = nullptr;
    buttonLeft[index].unlock = !lock;
}

void actionMapping::registerActionRight(uint8_t index, actionHandle handle, uint8_t lock) {
    buttonRight[index].handle = handle;
    buttonRight[index].data = nullptr;
    buttonRight[index].unlock = !lock;
}
void actionMapping::registerActionEncoder(uint8_t index, actionHandle handleCW, actionHandle handleCCW,
                                          actionHandle handlePUSH) {
    encoder[index].handle_CW = handleCW;
    encoder[index].handle_CCW = handleCCW;
    encoder[index].handle_PUSH = handlePUSH;
}

void actionMapping::callActionEncoder_CW(uint8_t index) {
    if (encoder[index].handle_CW.functionPointer != nullptr)
        encoder[index].handle_CW.functionPointer();
}

void actionMapping::callActionEncoder_CCW(uint8_t index) {
    if (encoder[index].handle_CCW.functionPointer != nullptr)
        encoder[index].handle_CCW.functionPointer();
}

void actionMapping::callActionEncoder_Push(uint8_t index) {
    if (encoder[index].handle_PUSH.functionPointer != nullptr)
        encoder[index].handle_PUSH.functionPointer();
}

void actionMapping::clear() {

    for (uint32_t i = 0; i < NUMBUTTONSIDE; i++) {
        buttonLeft[i].handle = {nullptr, ""};
        buttonLeft[i].data = nullptr;
        buttonLeft[i].unlock = 0;

        buttonRight[i].handle = {nullptr, ""};
        buttonRight[i].data = nullptr;
        buttonRight[i].unlock = 0;
    }
    for (uint32_t i = 0; i < NUMBUTTONTOP; i++) {
        buttonFooter[i] = {nullptr, ""};
    }
    for (uint32_t i = 0; i < NUMBUTTONBOTTOM; i++) {
        buttonHeader[i] = {nullptr, ""};
    }
    for (uint32_t i = 0; i < NUMENCODER; i++) {
        encoder[i].handle_CW = {nullptr, ""};
        encoder[i].handle_CCW = {nullptr, ""};
        encoder[i].handle_PUSH = {nullptr, ""};
    }
}

#endif