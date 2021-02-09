#ifdef POLYCONTROL
#include "liveDataBase.hpp"

bool compareByNote(const Key &a, const Key &b) {
    return a.note < b.note;
}

uint16_t clockTicksPerStep[23] = {1,  2,  3,  4,  6,  8,   9,   12,  16,  18,  24, 32,
                                  36, 48, 64, 72, 96, 128, 144, 192, 256, 288, 384};
#endif