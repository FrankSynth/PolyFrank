#pragma once

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

float calcSquircle(float value, float curvature);
float calcSquircleSimplified(float value, float curvature);

template <typename T> inline T getSign(T val) {
    return (T(0) <= val) - (val < T(0));
}

/**
 * @brief
 *
 * @param value between 0 and 1
 * @param curvature between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return float
 */

/**
 * @brief
 *
 * @param value [] between 0 and 1
 * @param curvature [] between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 */
ALWAYS_INLINE inline void calcSquircleSimplified(float *value, const float *curvature) {
    float exp[VOICESPERCHIP];
    // float expsq[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        exp[i] = 1.0f / curvature[i] - 1.0f;

    // for (uint32_t i = 0; i < VOICESPERCHIP; i++)
    //     expsq[i] = exp[i] * exp[i];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        value[i] = std::pow(value[i], exp[i]);
}

/**
 * @brief
 *
 * @param value [] between 0 and 1
 * @param curvature [] between 0.00001 and 0.99999, 0.5 = straight line, -> 1 bends upwards.
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline void calcSquircle(float *value, const float *curvature) {

    float sign[VOICESPERCHIP];
    float exp[VOICESPERCHIP];
    float firstpow[VOICESPERCHIP];

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        sign[i] = getSign(value[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        exp[i] = 1.0f / curvature[i] - 1.0f;

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        firstpow[i] = 1.0f - std::pow(sign[i] * value[i], exp[i]);

    for (uint32_t i = 0; i < VOICESPERCHIP; i++)
        value[i] = sign[i] * (1.0f - std::pow(firstpow[i], 1.0f / exp[i]));
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
 * @brief custom fast vector class for math operations
 *
 * @tparam Size size of array
 * @tparam A type, defaults to float
 */
template <uint32_t Size, typename A = float> class vec {

    A data[Size];

  public:
    vec() {}
    vec(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = other[i];
    }
    vec(A x) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = x;
    }
    // template <typename T> vec(const vec<Size, T> &other) {
    //     for (uint32_t i = 0; i < Size; i++)
    //         data[i] = (A)other[i];
    // }

    ~vec() {}

    A &operator[](int i) { return data[i]; }
    const A &operator[](int i) const { return data[i]; }

    operator A *() { return data; }
    operator const A *() const { return data; }

    template <typename T> operator vec<Size, T>() {
        vec<Size, T> newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = (T)data[i];
        return newVector;
    }
    template <typename T> operator const vec<Size, T>() const {
        vec<Size, T> newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = (T)data[i];
        return newVector;
    }

    vec &operator=(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = other[i];
        return *this;
    }
    // template <typename T> vec &operator=(const vec<Size, T> &other) {
    //     for (uint32_t i = 0; i < Size; i++)
    //         data[i] = (A)other[i];
    //     return *this;
    // }
    vec &operator=(A other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = other;
        return *this;
    }

    vec<Size, bool> operator<(const vec &other) const {
        vec<Size, bool> newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] < other[i];
        return newVector;
    }
    vec<Size, bool> operator<(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] < other;
        return newVector;
    }

    vec<Size, bool> operator<=(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] <= other[i];
        return newVector;
    }
    vec<Size, bool> operator<=(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] <= other;
        return newVector;
    }

    vec<Size, bool> operator>(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] > other[i];
        return newVector;
    }
    vec<Size, bool> operator>(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] > other;
        return newVector;
    }

    vec<Size, bool> operator>=(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] >= other[i];
        return newVector;
    }
    vec<Size, bool> operator>=(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] >= other;
        return newVector;
    }

    vec<Size, bool> operator==(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] == other[i];
        return newVector;
    }
    vec<Size, bool> operator==(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] == other;
        return newVector;
    }

    vec<Size, bool> operator!=(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] != other[i];
        return newVector;
    }
    vec<Size, bool> operator!=(A other) const {
        vec<Size, bool> newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] != other;
        return newVector;
    }

    vec<Size, bool> operator&&(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] && other[i];
        return newVector;
    }
    vec<Size, bool> operator&&(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] && other;
        return newVector;
    }

    vec<Size, bool> operator||(const vec &other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] || other[i];
        return newVector;
    }
    vec<Size, bool> operator||(A other) const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] || other;
        return newVector;
    }

    vec<Size, bool> operator!() const {
        vec<Size, bool> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = !data[i];
        return newVector;
    }

    vec<Size, uint32_t> operator&(const vec<Size, uint32_t> &other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] & other[i];
        return newVector;
    }
    vec<Size, uint32_t> operator&(uint32_t other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] & other;
        return newVector;
    }

    vec<Size, uint32_t> operator|(const vec<Size, uint32_t> &other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] | other[i];
        return newVector;
    }
    vec<Size, uint32_t> operator|(uint32_t other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] | other;
        return newVector;
    }

    vec<Size, uint32_t> operator<<(const vec<Size, uint32_t> &other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] << other[i];
        return newVector;
    }
    vec<Size, uint32_t> operator<<(uint32_t other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] << other;
        return newVector;
    }

    vec<Size, uint32_t> operator>>(const vec<Size, uint32_t> &other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] >> other[i];
        return newVector;
    }
    vec<Size, uint32_t> operator>>(uint32_t other) const {
        vec<Size, uint32_t> newVector;

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = data[i] >> other;
        return newVector;
    }

    vec operator+(const vec &other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] += other[i];
        return newVector;
    }
    vec operator+(A other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] += other;
        return newVector;
    }

    vec operator-(const vec<Size, A> &other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] -= other[i];
        return newVector;
    }
    vec operator-(A other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] -= other;
        return newVector;
    }

    vec operator*(const vec<Size, A> &other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] *= other[i];
        return newVector;
    }
    vec operator*(A other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] *= other;
        return newVector;
    }

    vec operator/(const vec &other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] /= other[i];
        return newVector;
    }
    vec operator/(A other) const {
        vec newVector(*this);

        for (uint32_t i = 0; i < Size; i++)
            newVector[i] /= other;
        return newVector;
    }

    vec operator-() const {
        vec newVector(*this);
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] *= (A)(-1);
        return newVector;
    }

    vec &operator+=(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] += other[i];
        return *this;
    }
    vec &operator+=(A other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] += other;
        return *this;
    }

    vec &operator-=(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] -= other[i];
        return *this;
    }
    vec &operator-=(A other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] -= other;
        return *this;
    }

    vec &operator*=(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] *= other[i];
        return *this;
    }
    vec &operator*=(A other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] *= other;
        return *this;
    }

    vec &operator/=(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] /= other[i];
        return *this;
    }
    vec &operator/=(A other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] /= other;
        return *this;
    }

    vec operator++(int) {
        vec newVector(*this);
        for (uint32_t i = 0; i < Size; i++)
            data[i]++;
        return newVector;
    }
    vec operator--(int) {
        vec newVector(*this);
        for (uint32_t i = 0; i < Size; i++)
            data[i]--;
        return newVector;
    }

    vec &operator++() {
        for (uint32_t i = 0; i < Size; i++)
            data[i]++;
        return *this;
    }
    vec &operator--() {
        for (uint32_t i = 0; i < Size; i++)
            data[i]--;
        return *this;
    }

    friend vec operator-(float x, const vec &v) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = x - v[i];
        return newVector;
    }
    friend vec operator/(float x, const vec &v) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = x / v[i];
        return newVector;
    }
    friend vec operator*(float x, const vec &v) { return v * x; }
    friend vec operator+(float x, const vec &v) { return v + x; }

    friend vec operator<<(uint32_t x, const vec &v) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = x / v[i];
        return newVector;
    }

    friend vec operator>>(uint32_t x, const vec &v) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = x / v[i];
        return newVector;
    }

    friend inline vec simpleBezier1D(const float p1, const vec &t) { return vec(2.0f * (1.0f - t) * t * p1 + t * t); }
    friend inline vec simpleBezier1D(const vec &p1, const vec &t) { return vec(2.0f * (1.0f - t) * t * p1 + t * t); }
    friend inline vec fast_lerp_f32(const vec &a, const vec &b, float f) { return vec(a * (1.0f - f) + (b * f)); }
    friend inline vec fast_lerp_f32(const vec &a, const vec &b, const vec &f) { return vec(a * (1.0f - f) + (b * f)); }

    friend inline vec min(const vec &a, const vec &b) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::min(a[i], b[i]);
        return newVector;
    }
    friend inline vec min(const vec &a, A b) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::min(a[i], b);
        return newVector;
    }

    friend inline vec max(const vec &a, const vec &b) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::max(a[i], b[i]);
        return newVector;
    }
    friend inline vec max(const vec &a, A b) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::max(a[i], b);
        return newVector;
    }

    friend inline vec clamp(const vec &a, const vec &lower, const vec &upper) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::clamp(a[i], lower[i], upper[i]);
        return newVector;
    }
    friend inline vec clamp(const vec &a, A lower, A upper) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::clamp(a[i], lower, upper);
        return newVector;
    }

    friend inline vec floor(const vec &a) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::floor(a[i]);
        return newVector;
    }

    friend inline vec ceil(const vec &a) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::ceil(a[i]);
        return newVector;
    }

    friend inline vec round(const vec &a) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::round(a[i]);
        return newVector;
    }

    friend inline vec getSign(const vec &a) { return vec((vec<Size, bool>(a >= (A)0)) - vec<Size, bool>((a < (A)0))); }
};

ALWAYS_INLINE inline vec<2, float> bezier2D(const vec<2, float> &p0, const vec<2, float> &p1, const vec<2, float> &p2,
                                            const float t) {
    return (vec<2, float>)((1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2);
}

ALWAYS_INLINE inline vec<2, float> simpleBezier2D(const vec<2, float> &p1, const float t) {
    return (vec<2, float>)((2.0f * (1.0f - t) * t) * p1 + (t * t));
}