#pragma once

#include "datacore/dataHelperFunctions.hpp"
#include "datacore/datadef.h"
#include "stdint.h"
#include <cmath>

/**
 * @brief custom fast vector class for math operations
 *
 * @tparam Size size of array
 * @tparam A type, defaults to float
 */
template <uint32_t Size, typename A = float> class vec {

  public:
    A data[Size];

    vec() {}
    vec(const vec &other) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = other[i];
    }
    vec(A x) {
        for (uint32_t i = 0; i < Size; i++)
            data[i] = x;
    }

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

    vec operator!() const {
        vec newVector;

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
            newVector[i] = x << v[i];
        return newVector;
    }

    friend vec operator>>(uint32_t x, const vec &v) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = x >> v[i];
        return newVector;
    }

    friend inline vec simpleBezier1D(const float p1, const vec &t) { return vec(2.0f * (1.0f - t) * t * p1 + t * t); }
    friend inline vec simpleBezier1D(const vec &p1, const vec &t) { return vec(2.0f * (1.0f - t) * t * p1 + t * t); }
    friend inline vec simpleCubicBezier1D(const vec &p1, const vec &t) {
        return vec(3.0f * t * (1.0f - t) * (1.0f - t) * p1 + 3.0f * t * t * (1 - t) * p1 + t * t * t);
    }
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
    friend inline vec fabs(const vec &a) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = std::fabs(a[i]);
        return newVector;
    }

    // returns 1 or -1
    friend inline vec getSign(const vec &a) {
        vec newVector;
        for (uint32_t i = 0; i < Size; i++)
            newVector[i] = (A)(a[i] >= (A)0) - (A)(a[i] < (A)0);
        return newVector;
    }
};