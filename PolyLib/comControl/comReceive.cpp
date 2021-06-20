#ifdef POLYCONTROL

#include "comReceive.hpp"

CircularBuffer<uint8_t, 512> receiveBuffer; // read buffer

void comReceive(uint8_t *Buf, uint32_t *Len) {
    for (uint32_t i = 0; i < *Len; i++) {
        receiveBuffer.push_back(Buf[i]);
    }
}

uint8_t comAvailable() {
    if (receiveBuffer.empty()) {
        return 0;
    }
    else {
        return 1;
    }
}

char comRead() {
    uint8_t data;
    if (!receiveBuffer.empty()) {
        data = receiveBuffer.front();
        receiveBuffer.pop_front();
    }
    else {
        return 0;
    }
    return data;
}

#endif