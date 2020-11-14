#pragma once

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
ALWAYS_INLINE inline int32_t changeInt(const int32_t value, const int32_t change, const int32_t minimum,
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
ALWAYS_INLINE inline int32_t changeIntLoop(const int32_t value, const int32_t change, const int32_t minimum,
                                           const int32_t maximum) {

    if (value + change > maximum) { // test max

        return minimum;
    }
    else if (value + change < minimum) { // test min
        return maximum;
    }
    else {
        return (value + change); // return new value
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
ALWAYS_INLINE inline int32_t testInt(const int32_t value, const int32_t minimum, const int32_t maximum) {
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
ALWAYS_INLINE inline float changeFloat(const float value, const float change, const float minimum,
                                       const float maximum) {

    if (value + change > maximum) { // test max

        return maximum;
    }
    else if (value + change < minimum) { // test min
        return minimum;
    }
    else {
        return (value + change); // return new value
    }
}

/**
 * @brief test float and return with value between min/max
 *
 * @param value
 * @param minimum
 * @param maximum
 * @return ALWAYS_INLINE
 */
ALWAYS_INLINE inline float testFloat(const float value, const float minimum, const float maximum) {
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
