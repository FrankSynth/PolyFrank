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
        noteLin2LogTable_f32[i] =
            std::pow(2.0f, fastMap(i, 0, FAST_NOTELIN2LOG_TABLE_SIZE, noteLin2logMIN, noteLin2logMAX));
    }
}

float fastNoteLin2Log_f32(float x) {
    float ret, fract; /* Temporary variables for input, output */
    uint16_t index;   /* Index variable */
    float a, b;       /* Two nearest output values */
    // int32_t n;
    float findex;

    x = fastMap(std::clamp(x, (float)noteLin2logMIN, (float)noteLin2logMAX), noteLin2logMIN, noteLin2logMAX, 0, 1);

    /* Calculation of index of the table */
    findex = (float)FAST_NOTELIN2LOG_TABLE_SIZE * x;

    index = ((uint16_t)findex);

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = noteLin2LogTable_f32[index];
    b = noteLin2LogTable_f32[index + 1];

    /* Linear interpolation process */
    ret = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    return (ret);
}

void LogCurve::precomputeTable() {
    float b = std::pow((1.0f / curve) - 1.0f, 2);
    float a = 1.0f / (b - 1.0f);

    for (uint16_t i = 0; i < (size + 1); i++) {
        logTable[i] = a * std::pow(b, fastMap(i, 0, size, 0, 1)) - a;
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
    uint32_t index;   /* Index variable */
    float a, b;       /* Two nearest output values */
    // int32_t n;
    float findex;

    /* Calculation of index of the table */
    findex = (float)size * std::clamp(value, 0.0f, 1.0f);

    index = ((uint32_t)findex);

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = logTable[index];
    b = logTable[index + 1];

    /* Linear interpolation process */
    ret = fast_lerp_f32(a, b, fract);

    /* Return the output value */

    return ret;
}

/**
 * @brief map values between -1 and 1 to specific log curve
 *
 * @param value
 * @return float
 */
float LogCurve::mapValueSigned(float value) {
    int32_t sign;

    if (value < 0) {
        sign = 1;
        value = -value;
    }
    else {
        sign = 0;
    }

    float ret, fract; /* Temporary variables for input, output */
    uint32_t index;   /* Index variable */
    float a, b;       /* Two nearest output values */
    // int32_t n;
    float findex;

    /* Calculation of index of the table */
    findex = (float)size * std::clamp(value, 0.0f, 1.0f);

    index = ((uint32_t)findex);

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = logTable[index];
    b = logTable[index + 1];

    /* Linear interpolation process */
    ret = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    if (sign == 0) {
        return ret;
    }
    else {
        return -ret;
    }
}

// audio poti Log style
LogCurve audioCurve(64, 0.1);

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
float calcSquircle(float value, float curvature) {
    if (curvature == 0.5f)
        return value;

    float sign = getSign(value);

    float exp = 1.0f / curvature - 1.0f;
    float firstpow = 1.0f - std::pow(sign * value, exp);
    return sign * (1.0f - std::pow(firstpow, 1.0f / exp));
}

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
float calcSquircleSimplified(float value, float curvature) {
    if (curvature == 0.5f)
        return value;

    float exp = 1.0f / curvature - 1.0f;

    float ret = std::pow(value, exp * exp);
    return ret;
}

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
vec<VOICESPERCHIP> calcSquircleSimplified(const vec<VOICESPERCHIP> &value, const vec<VOICESPERCHIP> &curvature) {
    vec<VOICESPERCHIP> exp = (1.0f / curvature) - 1.0f;
    return powf(value, exp * exp);
}
/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
vec<VOICESPERCHIP> calcSquircle(const vec<VOICESPERCHIP> &value, const vec<VOICESPERCHIP> &curvature) {
    vec<VOICESPERCHIP> exp = (1.0f / curvature) - 1.0f;
    vec<VOICESPERCHIP> firstpow = 1.0f - powf(value, exp);
    return (1.0f - powf(firstpow, 1.0f / exp));
}
