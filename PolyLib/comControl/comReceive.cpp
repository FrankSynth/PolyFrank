#ifdef POLYCONTROL

#include "comReceive.hpp"
#include "debughelper/debughelper.hpp"
CircularBuffer<uint8_t, 512> receiveBuffer; // read buffer

void comReceive(uint8_t *Buf, uint32_t *Len) {
    for (uint32_t i = 0; i < *Len; i++) {
        receiveBuffer.push_back(Buf[i]);
    }
}

uint8_t comAvailable() {
    return !receiveBuffer.empty();
}

char comRead() {
    uint8_t data;
    if (!receiveBuffer.empty()) {
        data = receiveBuffer.front();
        receiveBuffer.pop_front();
        return data;
    }
    return 0;
}

#endif