#include "polyMath.hpp"

float fast_sin_f32(float x) {
    float sinVal, fract; /* Temporary variables for input, output */
    uint16_t index;      /* Index variable */
    float a, b;          /* Two nearest output values */
    int32_t n;
    float findex;

    /* Special case for small negative inputs */
    if ((x < 0.0f) && (x >= -1.9e-7f)) {
        return x;
    }

    /* input x is in radians */
    /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
    // in = x * 0.159154943092f;

    /* Calculation of floor value of input */
    n = (int32_t)x;

    /* Make negative values towards -infinity */
    if (x < 0.0f) {
        n--;
    }

    /* Map input value to [0 1] */
    x = x - (float)n;

    /* Calculation of index of the table */
    findex = (float)FAST_MATH_TABLE_SIZE * x;

    index = ((uint16_t)findex) & 0x1ff;

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = sinTable_f32[index];
    b = sinTable_f32[index + 1];

    /* Linear interpolation process */
    // sinVal = (1.0f - fract) * a + fract * b;
    sinVal = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    return (sinVal);
}

float noteLin2LogTable_f32[FAST_NOTELIN2LOG_TABLE_SIZE + 1];
void precomputeNoteLin2LogTable() {
    for (uint32_t i = 0; i < FAST_NOTELIN2LOG_TABLE_SIZE + 1; i++) {
        noteLin2LogTable_f32[i] = powf(2.0, fastMap(i, 0, FAST_NOTELIN2LOG_TABLE_SIZE, noteLin2logMIN, noteLin2logMAX));
    }
}

float fastNoteLin2Log_f32(float x) {
    float ret, fract; /* Temporary variables for input, output */
    uint16_t index;   /* Index variable */
    float a, b;       /* Two nearest output values */
    // int32_t n;
    float findex;

    x = fastMap(testFloat(x, noteLin2logMIN, noteLin2logMAX), noteLin2logMIN, noteLin2logMAX, 0, 1);

    /* Calculation of index of the table */
    findex = (float)FAST_NOTELIN2LOG_TABLE_SIZE * x;

    index = ((uint16_t)findex);

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = noteLin2LogTable_f32[index];
    b = noteLin2LogTable_f32[index + 1];

    /* Linear interpolation process */
    // ret = (1.0f - fract) * a + fract * b;
    ret = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    return (ret);
}

void LogCurve::precomputeTable() {
    float b = std::pow((1 / curve) - 1, 2);
    float a = 1 / (b - 1);

    for (uint16_t i = 0; i < (size + 1); i++) {
        logTable[i] = a * std::pow(b, fastMapCached(i, 0, size, 0, 1)) - a;
        // logTable.push_back(a * powf(b, fastMap(i, 0, size, 0, 1)) - a);
    }
}

/**
 * @brief map values between 0 and 1 to specific log curve
 *
 * @param value
 * @return float
 */
float LogCurve::mapValue(float value) {
    float ret, fract; /* Temporary variables for input, output */
    uint16_t index;   /* Index variable */
    float a, b;       /* Two nearest output values */
    // int32_t n;
    float findex;

    /* Calculation of index of the table */
    findex = (float)size * testFloat(value, 0, 1);

    index = ((uint16_t)findex);

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = logTable[index];
    b = logTable[index + 1];

    /* Linear interpolation process */
    ret = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    return (ret);
}

// audio poti Log style
LogCurve audioCurve(16, 0.1);

float fastMapAudioToLog(float value) {
    return audioCurve.mapValue(value);
}