#pragma once

#include "tim.h"
#include <cmath>
#include <random>
#include <stdint.h>

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

/**
 * @brief change int with min or max
 *
 * @param value
 * @param change
 * @param minimum
 * @param maximum
 * @param clampChange = true
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline const int32_t changeInt(const int32_t value, const int32_t change, const int32_t minimum,
                                             const int32_t maximum, const bool clampChange = true) {

    if (value + change > maximum) { // test max

        return clampChange ? maximum : value;
    }
    else if (value + change < minimum) { // test min
        return clampChange ? minimum : value;
    }
    else {
        return (value + change); // return new value
    }
}

/**
 * @brief change int with rollover
 *
 * @param value
 * @param change
 * @param minimum
 * @param maximum
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline const int32_t changeIntLoop(const int32_t value, const int32_t change, const int32_t minimum,
                                                 const int32_t maximum) {
    int32_t changed = value + change;

    if (changed > maximum) { // test max

        return minimum;
    }
    else if (changed < minimum) { // test min
        return maximum;
    }
    else {
        return changed; // return new value
    }
}

/**
 * @brief test int and return with value between min/max
 *
 * @param value
 * @param minimum
 * @param maximum
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline const int32_t testInt(const int32_t value, const int32_t minimum, const int32_t maximum) {
    if (value > maximum) {
        return maximum;
    }
    else if (value < minimum) {
        return minimum;
    }
    else {
        return value;
    }
}

/**
 * @brief change float with rspect to min or max
 *
 * @param value
 * @param change
 * @param minimum
 * @param maximum
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline const float changeFloat(const float value, const float change, const float minimum,
                                             const float maximum) {

    return std::clamp(value + change, minimum, maximum);
}

/**
 * @brief test float and return with value between min/max
 *
 * @param value
 * @param minimum
 * @param maximum
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline const float testFloat(const float value, const float minimum, const float maximum) {
    if (value > maximum) {
        return maximum;
    }
    else if (value < minimum) {
        return minimum;
    }
    else {
        return value;
    }
}

inline uint32_t micros() {
    return __HAL_TIM_GetCounter(&htim2);
}

inline uint32_t millis() {
    return HAL_GetTick();
}

/* Elapsed time types - for easy-to-use measurements of elapsed time
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2011 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

class elapsedMicros {
  private:
    uint32_t us;

  public:
    elapsedMicros(void) { us = micros(); }
    elapsedMicros(uint32_t val) { us = micros() - val; }
    elapsedMicros(const elapsedMicros &orig) { us = orig.us; }
    operator uint32_t() const { return micros() - us; }
    elapsedMicros &operator=(const elapsedMicros &rhs) {
        us = rhs.us;
        return *this;
    }
    elapsedMicros &operator=(uint32_t val) {
        us = micros() - val;
        return *this;
    }
    elapsedMicros &operator-=(uint32_t val) {
        us += val;
        return *this;
    }
    elapsedMicros &operator+=(uint32_t val) {
        us -= val;
        return *this;
    }
    elapsedMicros operator-(int32_t val) const {
        elapsedMicros r(*this);
        r.us += val;
        return r;
    }
    elapsedMicros operator-(uint32_t val) const {
        elapsedMicros r(*this);
        r.us += val;
        return r;
    }
    elapsedMicros operator+(int32_t val) const {
        elapsedMicros r(*this);
        r.us -= val;
        return r;
    }
    elapsedMicros operator+(uint32_t val) const {
        elapsedMicros r(*this);
        r.us -= val;
        return r;
    }
};

class elapsedMillis {
  private:
    uint32_t ms;

  public:
    elapsedMillis(void) { ms = millis(); }
    elapsedMillis(uint32_t val) { ms = millis() - val; }
    elapsedMillis(const elapsedMillis &orig) { ms = orig.ms; }
    operator uint32_t() const { return millis() - ms; }
    elapsedMillis &operator=(const elapsedMillis &rhs) {
        ms = rhs.ms;
        return *this;
    }
    elapsedMillis &operator=(uint32_t val) {
        ms = millis() - val;
        return *this;
    }
    elapsedMillis &operator-=(uint32_t val) {
        ms += val;
        return *this;
    }
    elapsedMillis &operator+=(uint32_t val) {
        ms -= val;
        return *this;
    }
    elapsedMillis operator-(int32_t val) const {
        elapsedMillis r(*this);
        r.ms += val;
        return r;
    }
    elapsedMillis operator-(uint32_t val) const {
        elapsedMillis r(*this);
        r.ms += val;
        return r;
    }
    elapsedMillis operator+(int32_t val) const {
        elapsedMillis r(*this);
        r.ms -= val;
        return r;
    }
    elapsedMillis operator+(uint32_t val) const {
        elapsedMillis r(*this);
        r.ms -= val;
        return r;
    }
};

class elapsedSeconds {
  private:
    uint32_t s;

  public:
    elapsedSeconds(void) { s = millis() / 1000; }
    elapsedSeconds(uint32_t val) { s = millis() / 1000 - val; }
    elapsedSeconds(const elapsedSeconds &orig) { s = orig.s; }
    operator uint32_t() const { return millis() / 1000 - s; }
    elapsedSeconds &operator=(const elapsedSeconds &rhs) {
        s = rhs.s;
        return *this;
    }
    elapsedSeconds &operator=(uint32_t val) {
        s = millis() / 1000 - val;
        return *this;
    }
    elapsedSeconds &operator-=(uint32_t val) {
        s += val;
        return *this;
    }
    elapsedSeconds &operator+=(uint32_t val) {
        s -= val;
        return *this;
    }
    elapsedSeconds operator-(int32_t val) const {
        elapsedSeconds r(*this);
        r.s += val;
        return r;
    }
    elapsedSeconds operator-(uint32_t val) const {
        elapsedSeconds r(*this);
        r.s += val;
        return r;
    }
    elapsedSeconds operator+(int32_t val) const {
        elapsedSeconds r(*this);
        r.s -= val;
        return r;
    }
    elapsedSeconds operator+(uint32_t val) const {
        elapsedSeconds r(*this);
        r.s -= val;
        return r;
    }
};

/**
 * @brief Copies the elements of a floating-point vector.
 * @param[in]       *pSrc points to input vector
 * @param[out]      *pDst points to output vector
 * @param[in]       length of the input vector in word
 * @return none.
 *
 */
ALWAYS_INLINE inline void fast_copy_f32(uint32_t *pSrc, uint32_t *pDst, uint32_t blockSize) {
    uint32_t blkCnt; /* loop counter */

    uint32_t in1, in2, in3, in4;

    /*loop Unrolling */
    blkCnt = blockSize >> 2;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */
        in1 = *pSrc++;
        in2 = *pSrc++;
        in3 = *pSrc++;
        in4 = *pSrc++;

        *pDst++ = in1;
        *pDst++ = in2;
        *pDst++ = in3;
        *pDst++ = in4;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize & 0x03;

    while (blkCnt > 0) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */
        *pDst++ = *pSrc++;

        /* Decrement the loop counter */
        blkCnt--;
    }
}

ALWAYS_INLINE inline void fastMemset(uint32_t *data, uint32_t *pDst, uint32_t blockSize) {
    uint32_t blkCnt; /* loop counter */

    /*loop Unrolling */
    blkCnt = blockSize >> 2U;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */

        *pDst++ = *data;
        *pDst++ = *data;
        *pDst++ = *data;
        *pDst++ = *data;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */
        *pDst++ = *data;

        /* Decrement the loop counter */
        blkCnt--;
    }
}

inline void microsecondsDelay(const uint32_t delay) {
    uint32_t time = __HAL_TIM_GetCounter(&htim2);
    while (__HAL_TIM_GetCounter(&htim2) - time < delay) {
        ;
    }
}

inline float calcRandom() {
    uint32_t randomNumber;

    randomNumber = std::rand();
    randomNumber = randomNumber & 0x00FFFFFF;

    // map to -1, 1
    return ((float)randomNumber / 8388607.0f) - 1.0f;
}