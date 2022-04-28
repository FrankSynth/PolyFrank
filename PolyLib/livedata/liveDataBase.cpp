#ifdef POLYCONTROL
#include "liveDataBase.hpp"

bool compareByNote(const Key &a, const Key &b) {
    return a.note < b.note;
}

#define DOT 1.0f / 1.5f
#define TRI 3.0f / 2.0f

#define QUARTER 1.0f
#define EIGHT 2.0f * QUARTER
#define SIXTEENTH 2.0f * EIGHT
#define THIRTYTWOTH 2.0f * SIXTEENTH
#define SIXTYFOURTH 2.0f * THIRTYTWOTH
#define HALF QUARTER / 2.0f
#define FULL HALF / 2.0f
#define TWOFULL FULL / 2.0f
#define FOURFULL TWOFULL / 2.0f

const uint32_t clockTicksPerStep[23] = {1,  2,  3,  4,  6,  8,   9,   12,  16,  18,  24, 32,
                                        36, 48, 64, 72, 96, 128, 144, 192, 256, 288, 384};
const float multTime[23] = {SIXTYFOURTH * TRI,
                            THIRTYTWOTH *TRI,
                            THIRTYTWOTH,
                            SIXTEENTH *TRI,
                            SIXTEENTH,
                            EIGHT *TRI,
                            SIXTEENTH *DOT,
                            EIGHT,
                            QUARTER *TRI,
                            EIGHT *DOT,
                            QUARTER,
                            HALF *TRI,
                            QUARTER *DOT,
                            HALF,
                            FULL *TRI,
                            HALF *DOT,
                            FULL,
                            TWOFULL *TRI,
                            FULL *DOT,
                            TWOFULL,
                            FOURFULL *TRI,
                            TWOFULL *DOT,
                            FOURFULL};
#endif