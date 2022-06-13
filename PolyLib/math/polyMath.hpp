#pragma once

#include "PolyVector.hpp"
#include "datacore/dataHelperFunctions.hpp"
#include "datacore/datadef.h"
#include "stdint.h"
#include <cmath>
#include <valarray>
#include <vector>

// #include "string.h"

#define FAST_MATH_TABLE_SIZE 512
#define noteLin2logMIN -2
#define noteLin2logMAX 9
#define FAST_NOTELIN2LOG_TABLE_SIZE (noteLin2logMAX - noteLin2logMIN) * 12 // 12 notes per octave

#define LEDCURVEPOINTSTART 0.0f // lowest brightness
#define LEDCURVEPOINT1IN 0.5f
#define LEDCURVEPOINT1OUT 0.25f // tune this
#define LEDCURVEPOINT2IN 0.9f
#define LEDCURVEPOINT2OUT 0.5f // tune this
#define LEDCURVEPOINTEND 1.0f  // max brightness

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

// table for fast sin
extern const float sinTable_f32[FAST_MATH_TABLE_SIZE + 1];

// table for fast lin2log
extern float noteLin2LogTable_f32[FAST_NOTELIN2LOG_TABLE_SIZE + 1];

/**
 * @brief  Floating-point sin_cos function.
 * @param[in]  theta   input value from 0 to 1 = 1 cycle, not degree
 * @param[out] pSinVal  points to the processed sine output.
 * @param[out] pCosVal  points to the processed cos output.
 */
float fast_sin_f32(float x);

/**
 * @brief fast lerp with float
 *
 * @param a value a
 * @param b value a
 * @param f fraction between a and b
 * @return
 */
ALWAYS_INLINE inline float fast_lerp_f32(float a, float b, float f) {
    return (a * (1.0f - f)) + (b * f);
}

/**
 * @brief returns -1 for val>0 and 1 for val<=0
 *
 * @tparam T
 * @param val
 * @return T
 */
template <typename T> inline T getSign(T val) {
    return (T(0) <= val) - (val < T(0));
}

/**
 * @brief fast lerp with pointer float
 *
 * @param a value a
 * @param b value a
 * @param f fraction between a and b
 * @param dst dst pointer
 * @return
 */
ALWAYS_INLINE inline void fast_lerp_f32(const float *a, const float *b, const float *f, float *dst) {
    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        dst[i] = a[i] * (1.0f - f[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        dst[i] += b[i] * f[i];
}

/**
 * @brief approx lin2exp, to be tested!
 *
 * @param x
 * @param min
 * @param max
 * @return float
 */
inline float fastPowLin2Exp(float x, float min, float max) {
    return min + (powf(max + 1.0f, x / max) - 1.0f) * (max - min) / max;
}

/**
 * @brief approx lin2 exp without min, min is always 0, to be tested
 *
 * @param x
 * @param max
 * @return float
 */
inline float fastPowLin2Exp(float x, float max) {
    return std::pow(max + 1.0f, x / max) - 1.0f;
}

// /**
//  * @brief caches the calculated slope for similar consecutive map calls
//  *
//  * @param input input value to be mapped in range
//  * @param input_start in range start
//  * @param input_end in range end
//  * @param output_start out range start
//  * @param output_end out range end
//  * @return float mapped input value
//  */
// inline float fastMapCached(float input, float input_start, float input_end, float output_start, float output_end) {

//     static float input_start_store = 0;
//     static float input_end_store = 0;
//     static float output_start_store = 0;
//     static float output_end_store = 0;
//     static float slope_store = 0;

//     if (input_start != input_start_store || input_end != input_end_store || output_start != output_start_store ||
//         output_end != output_end_store) {
//         slope_store = (output_end - output_start) / (input_end - input_start);
//         input_start_store = input_start;
//         input_end_store = input_end;
//         output_start_store = output_start;
//         output_end_store = output_end;
//     }

//     float output = output_start + slope_store * (input - input_start);
//     return output;
// }

/**
 * @brief map value without caching the slope
 *
 * @param input input value to be mapped in range
 * @param input_start in range start
 * @param input_end in range end
 * @param output_start out range start
 * @param output_end out range end
 * @return float mapped input value
 */
ALWAYS_INLINE inline float fastMap(float input, float input_start, float input_end, float output_start,
                                   float output_end) {

    float output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
    return output;
}
ALWAYS_INLINE inline vec<VOICESPERCHIP> fastMap(const vec<VOICESPERCHIP> &input, const vec<VOICESPERCHIP> &input_start,
                                                const vec<VOICESPERCHIP> &input_end,
                                                const vec<VOICESPERCHIP> &output_start,
                                                const vec<VOICESPERCHIP> &output_end) {

    vec<VOICESPERCHIP> output =
        output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
    return output;
}

/**
 * @brief precompute lin2log tasble based on noteLin2logMIN, noteLin2logMAX
 *
 */
void precomputeNoteLin2LogTable();

/**
 * @brief convert Note values to pow(2,x)
 *
 * @param x range from noteLin2logMIN, lognoteLin2logMAX
 * @return float
 */
float fastNoteLin2Log_f32(float x);

/**
 * @brief custom led brightness curve
 *
 * @param value between 0 and 1
 * @return float mapped value between 0 and 1
 */
inline float fastMapLEDBrightness(float value) {
    // TODO bezier
    if (value < LEDCURVEPOINT1IN) {
        value = fastMap(value, LEDCURVEPOINTSTART, LEDCURVEPOINT1IN, LEDCURVEPOINTSTART, LEDCURVEPOINT1OUT);
    }
    else {
        if (value < LEDCURVEPOINT2IN) {
            value = fastMap(value, LEDCURVEPOINT1IN, LEDCURVEPOINT2IN, LEDCURVEPOINT1OUT, LEDCURVEPOINT2OUT);
        }
        else {
            value = fastMap(value, LEDCURVEPOINT2IN, LEDCURVEPOINTEND, LEDCURVEPOINT2OUT, LEDCURVEPOINTEND);
        }
    }
    return value;
}

/**
 * @brief custom log table between 0 and 1 with curve param
 *
 */
class LogCurve {
  public:
    /**
     * @brief Construct a new Log Curve object
     *
     * @param size
     * @param curve midpoint, between 0 and 1, 0.5 = linear
     */
    LogCurve(uint16_t size, float curve) {
        this->size = size;
        this->curve = std::clamp(curve == 0.5f ? 0.4999f : curve, 0.0001f, 0.9999f);
        this->logTable.reserve(size + 1);

        precomputeTable();
    }

    float mapValue(float value);
    float mapValueSigned(float value);
    void precomputeTable();

    /**
     * @brief map values between 0 and 1 to specific log curve
     *
     * @param value
     * @return float
     */
    template <uint32_t Size> vec<Size, float> mapValue(vec<Size, float> value) {

        vec<Size> ret, fract;      /* Temporary variables for input, output */
        vec<Size, uint32_t> index; /* Index variable */
        vec<Size> a, b;            /* Two nearest output values */
        // int32_t n;
        vec<Size> findex;

        /* Calculation of index of the table */
        findex = (vec<Size>)size * clamp(value, 0.0f, 1.0f);

        index = ((vec<Size, uint32_t>)findex);

        /* fractional value calculation */
        fract = findex - (vec<Size>)index;

        /* Read two nearest values of input value from the sin table */
        for (uint32_t i = 0; i < Size; i++) {

            a[i] = logTable[index[i]];
            b[i] = logTable[index[i] + 1];
        }

        /* Linear interpolation process */
        ret = fast_lerp_f32(a, b, fract);

        /* Return the output value */

        return ret;
    }

  private:
    float curve;
    uint16_t size;
    std::vector<float> logTable;
};

// audio poti Log style
extern LogCurve audioCurve;

/**
 * @brief map Audio log
 *
 * @param value between 0 and 1
 * @return float mapped value between 0 and 1
 */
inline float fastMapAudioToLog(float value) {
    return audioCurve.mapValue(value);
}

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
inline float calcSquircle(float value, float curvature) {
    if (curvature == 0.5f)
        return value;

    float exp = 1.0f / curvature - 1.0f;
    float firstpow = 1.0f - std::pow(value, exp);
    return (1.0f - std::pow(firstpow, 1.0f / exp));
}

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */
inline float calcSquircleSimplified(float value, float curvature) {
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
inline vec<VOICESPERCHIP> calcSquircleSimplified(const vec<VOICESPERCHIP> &value, const vec<VOICESPERCHIP> &curvature) {
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
inline vec<VOICESPERCHIP> calcSquircle(const vec<VOICESPERCHIP> &value, const vec<VOICESPERCHIP> &curvature) {
    vec<VOICESPERCHIP> exp = (1.0f / curvature) - 1.0f;
    vec<VOICESPERCHIP> firstpow = 1.0f - powf(value, exp);
    return (1.0f - powf(firstpow, 1.0f / exp));
}

/**
 * @brief define all three points, calcs just one dimension
 *
 * @param p0
 * @param p1
 * @param p2
 * @param t
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float bezier1D(const float p0, const float p1, const float p2, const float t) {
    return (1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2;
}

/**
 * @brief p0 = 0 and p2 = 1
 *
 * @param p1
 * @param t
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float simpleBezier1D(const float p1, const float t) {
    return 2.0f * (1.0f - t) * t * p1 + t * t;
}

/**
 * @brief p0 = 0 and p2 = 1
 *
 * @param p1
 * @param t
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float reverseSimpleBezier1D(const float v, const float p1) {
    float div = (2.0f * p1 - 1.0f);
    if (div == 0.0f)
        return v;
    return (p1 - sqrtf(p1 - 2.0f * p1 * v + v)) / (div);
}

ALWAYS_INLINE inline float reverseBezier1D(const float v, const float p0, const float p1, const float p2) {
    float div = (-p0 + 2.0f * p1 - p2);
    if (div == 0.0f)
        return fastMap(v, p0, p2, 0.0f, 1.0f);

    float sign;
    if (p0 < p2)
        sign = -1.0f;
    else
        sign = 1.0f;

    return (sign * sqrtf(-p0 * p2 + p0 * v + p1 * p1 - 2.0f * p1 * v + p2 * v) - p0 + p1) / (div);
}

/**
 * @brief p0 = 0 and p3 = 1
 *
 * @param p1 = p2
 * @param t
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float simpleCubicBezier1D(const float p1, const float t) {
    return 3.0f * t * (1.0f - t) * (1.0f - t) * p1 + 3.0f * t * t * (1 - t) * p1 + t * t * t;
}

ALWAYS_INLINE inline vec<2, float> bezier2D(const vec<2, float> &p0, const vec<2, float> &p1, const vec<2, float> &p2,
                                            const float t) {
    return (vec<2, float>)((1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2);
}

ALWAYS_INLINE inline vec<2, float> simpleBezier2D(const vec<2, float> &p1, const float t) {
    return (vec<2, float>)((2.0f * (1.0f - t) * t) * p1 + (t * t));
}
