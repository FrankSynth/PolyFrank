#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "stdint.h"
#include <cmath>
#include <vector>

// #include "string.h"

#define FAST_MATH_TABLE_SIZE 512
#define FAST_MATH_TABLE_SIZE 512
#define noteLin2logMIN -2
#define noteLin2logMAX 9
#define FAST_NOTELIN2LOG_TABLE_SIZE (noteLin2logMAX - noteLin2logMIN) * 12 // 12 notes per octave

#define LEDCURVEPOINTSTART 0.0f // lowest brightness
#define LEDCURVEPOINT1IN 0.5f
#define LEDCURVEPOINT1OUT 0.2f // tune this
#define LEDCURVEPOINT2IN 0.85f
#define LEDCURVEPOINT2OUT 0.5f // tune this
#define LEDCURVEPOINTEND 1.0f  // max brightness

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

/**
 * @brief  Process function for the floating-point Linear Interpolation Function.
 * @param[in,out] S  is an instance of the floating-point Linear Interpolation structure
 * @param[in]     x  input sample to process
 * @return y processed output sample.
 *
 */
typedef struct {
    uint32_t nValues; /**< nValues */
    float x1;         /**< x1 */
    float xSpacing;   /**< xSpacing */
    float *pYData;    /**< pointer to the table of Y values */
} arm_linear_interp_instance_f32;

// table for fast sin
extern const float sinTable_f32[FAST_MATH_TABLE_SIZE + 1];

// table for fast lin2log
extern float lin2logTable_f32[FAST_NOTELIN2LOG_TABLE_SIZE + 1];

/**
 * @brief  Floating-point sin_cos function.
 * @param[in]  theta   input value in degrees
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
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float fast_lerp_f32(float a, float b, float f) {
    return a + f * (b - a);
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
    return min + (powf(max + 1., x / max) - 1) * (max - min) / max;
}

/**
 * @brief approx lin2 exp without min, min is always 0, to be tested
 *
 * @param x
 * @param max
 * @return float
 */
inline float fastPowLin2Exp(float x, float max) {
    return powf(max + 1, x / max) - 1;
}

/**
 * @brief caches the calculated slope for similar consecutive map calls
 *
 * @param input input value to be mapped in range
 * @param input_start in range start
 * @param input_end in range end
 * @param output_start out range start
 * @param output_end out range end
 * @return float mapped input value
 */
inline float fastMapCached(float input, float input_start, float input_end, float output_start, float output_end) {

    static float input_start_store, input_end_store, output_start_store, output_end_store, slope_store = 0;

    if (input_start != input_start_store && input_end != input_end_store && output_start != output_start_store &&
        output_end != output_end_store) {
        slope_store = 1.0 * (output_end - output_start) / (input_end - input_start);
        input_start_store = input_start;
        input_end_store = input_end;
        output_start_store = output_start;
        output_end_store = output_end;
    }

    float output = output_start + slope_store * (input - input_start);
    return output;
}

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
inline float fastMap(float input, float input_start, float input_end, float output_start, float output_end) {

    float output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
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
 * @brief map Audio log
 *
 * @param value between 0 and 1
 * @return float mapped value between 0 and 1
 */
float fastMapAudioToLog(float value);

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
        this->curve = testFloat(curve == 0.5f ? 0.4999f : curve, 0.0001f, 0.9999f);
        this->logTable.reserve(size + 1);

        precomputeTable();
    }

    float mapValue(float value);

  private:
    float curve;
    uint16_t size;
    std::vector<float> logTable;

    void precomputeTable();
};