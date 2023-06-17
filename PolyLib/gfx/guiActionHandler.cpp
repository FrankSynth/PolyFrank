
#ifdef POLYCONTROL

#include "guiActionHandler.hpp"

actionMapping actionHandler;

void actionMapping::registerActionHeader(uint32_t index, actionHandle handle) {
    buttonHeader[index] = handle;
}

void actionMapping::registerActionFooter(uint32_t index, actionHandle handle) {
    buttonFooter[index] = handle;
}

void actionMapping::registerActionLeftData(uint32_t index, actionHandle handle, int32_t *data, uint32_t lock) {
    buttonLeft[index].handle = handle;
    buttonLeft[index].data = data;
    buttonLeft[index].unlock = !lock;
}

void actionMapping::registerActionRightData(uint32_t index, actionHandle handle, int32_t *data, uint32_t lock) {
    buttonRight[index].handle = handle;
    buttonRight[index].data = data;
    buttonRight[index].unlock = !lock;
}

void actionMapping::registerActionLeft(uint32_t index, actionHandle handle, uint32_t lock) {
    buttonLeft[index].handle = handle;
    buttonLeft[index].data = nullptr;
    buttonLeft[index].unlock = !lock;
}

void actionMapping::registerActionRight(uint32_t index, actionHandle handle, uint32_t lock) {
    buttonRight[index].handle = handle;
    buttonRight[index].data = nullptr;
    buttonRight[index].unlock = !lock;
}
void actionMapping::registerActionEncoder(uint32_t index, actionHandle handleCW, actionHandle handleCCW,
                                          actionHandle handlePUSH) {
    encoder[index].handle_CW = handleCW;
    encoder[index].handle_CCW = handleCCW;
    encoder[index].handle_PUSH = handlePUSH;
}

void actionMapping::callActionEncoder_CW(uint32_t index) {
    encoder[index].handle_CW.pendingCalls++;
}

void actionMapping::callActionEncoder_CCW(uint32_t index) {
    encoder[index].handle_CCW.pendingCalls++;
}

void actionMapping::callActionEncoder_Push(uint32_t index) {
    encoder[index].handle_PUSH.pendingCalls++;
}

void actionMapping::callActionFooter(uint32_t index) {
    buttonFooter[index].pendingCalls++;
}
void actionMapping::callActionHeader(uint32_t index) {
    buttonHeader[index].pendingCalls++;
}
void actionMapping::callActionLeft(uint32_t index) {
    buttonRight[index].handle.pendingCalls++;
}
void actionMapping::callActionRight(uint32_t index) {
    buttonLeft[index].handle.pendingCalls++;
}

void actionMapping::clear() {

    for (uint32_t i = 0; i < NUMBUTTONSIDE; i++) {
        buttonLeft[i].handle = {nullptr, "", 0};
        buttonLeft[i].data = nullptr;
        buttonLeft[i].unlock = 0;

        buttonRight[i].handle = {nullptr, "", 0};
        buttonRight[i].data = nullptr;
        buttonRight[i].unlock = 0;
    }
    for (uint32_t i = 0; i < NUMBUTTONTOP; i++) {
        buttonFooter[i] = {nullptr, "", 0};
    }
    for (uint32_t i = 0; i < NUMBUTTONBOTTOM; i++) {
        buttonHeader[i] = {nullptr, "", 0};
    }
    for (uint32_t i = 0; i < NUMENCODER; i++) {
        encoder[i].handle_CW = {nullptr, "", 0};
        encoder[i].handle_CCW = {nullptr, "", 0};
        encoder[i].handle_PUSH = {nullptr, "", 0};
    }
}

void actionMapping::ActionHandlerServiceRoutine() {

    for (uint32_t i = 0; i < NUMBUTTONSIDE; i++) {

        while (buttonLeft[i].handle.pendingCalls) {
            if (buttonLeft[i].handle.functionPointer != nullptr)
                buttonLeft[i].handle.functionPointer();
            buttonLeft[i].handle.pendingCalls--;
        }

        while (buttonRight[i].handle.pendingCalls) {
            if (buttonRight[i].handle.functionPointer != nullptr)
                buttonRight[i].handle.functionPointer();
            buttonRight[i].handle.pendingCalls--;
        }
    }
    for (uint32_t i = 0; i < NUMBUTTONTOP; i++) {

        while (buttonFooter[i].pendingCalls) {
            if (buttonFooter[i].functionPointer != nullptr)
                buttonFooter[i].functionPointer();
            buttonFooter[i].pendingCalls--;
        }
    }
    for (uint32_t i = 0; i < NUMBUTTONBOTTOM; i++) {

        while (buttonHeader[i].pendingCalls) {
            if (buttonHeader[i].functionPointer != nullptr)
                buttonHeader[i].functionPointer();
            buttonHeader[i].pendingCalls--;
        }
    }
    for (uint32_t i = 0; i < NUMENCODER; i++) {

        while (encoder[i].handle_CCW.pendingCalls) {
            if (encoder[i].handle_CCW.functionPointer != nullptr)
                encoder[i].handle_CCW.functionPointer();
            encoder[i].handle_CCW.pendingCalls--;
        }
        while (encoder[i].handle_CW.pendingCalls) {
            if (encoder[i].handle_CW.functionPointer != nullptr)
                encoder[i].handle_CW.functionPointer();
            encoder[i].handle_CW.pendingCalls--;
        }
        while (encoder[i].handle_PUSH.pendingCalls) {
            if (encoder[i].handle_PUSH.functionPointer != nullptr)
                encoder[i].handle_PUSH.functionPointer();
            encoder[i].handle_PUSH.pendingCalls--;
        }
    }
}

#endif