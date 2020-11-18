#pragma once

#include "math.h"
#include "stdint.h"
#include "string.h"

#define FAST_MATH_TABLE_SIZE 512
#define FAST_MATH_TABLE_SIZE 512
#define FAST_LIN2LOG_TABLE_SIZE 8192
#define lin2logMIN -1
#define lin2logMAX 9

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
extern const float lin2logTable_f32[FAST_LIN2LOG_TABLE_SIZE + 1];

/**
 * @brief  Floating-point sin_cos function.
 * @param[in]  theta   input value in degrees
 * @param[out] pSinVal  points to the processed sine output.
 * @param[out] pCosVal  points to the processed cos output.
 */

float fast_sin_f32(float x);

/**
 * @brief Copies the elements of a floating-point vector.
 * @param[in]       *pSrc points to input vector
 * @param[out]      *pDst points to output vector
 * @param[in]       blockSize length of the input vector
 * @return none.
 *
 */
inline void fast_copy_f32(float *pSrc, float *pDst, __uint32_t blockSize);

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
inline float fastLin2Exp(float x, float min, float max) {
    return min + (powf(max + 1., x / max) - 1) * (max - min) / max;
}

/**
 * @brief approx lin2 exp without min, min is always 0, to be tested
 *
 * @param x
 * @param max
 * @return float
 */
inline float fastLin2Exp(float x, float max) {
    return powf(max + 1, x / max) - 1;
}

void precomputeLin2LogTable();

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
inline float fastMap(float input, float input_start, float input_end, float output_start, float output_end) {

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