/*
 * spline.h
 *
 * simple cubic spline interpolation library without external
 * dependencies
 *
 * ---------------------------------------------------------------------
 * Copyright (C) 2011, 2014, 2016, 2021 Tino Kluge (ttk448 at gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ---------------------------------------------------------------------
 *
 */

#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#ifdef HAVE_SSTREAM
#include <sstream>
#include <string>
#endif // HAVE_SSTREAM

// not ideal but disable unused-function warnings
// (we get them because we have implementations in the header file,
// and this is because we want to be able to quickly separate them
// into a cpp file if necessary)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// unnamed namespace only because the implementation is in this
// header file and we don't want to export symbols to the obj files

namespace tk {

// spline interpolation
class spline {
  public:
    // spline types
    enum spline_type {
        linear = 10,         // linear interpolation
        cspline = 30,        // cubic splines (classical C^2)
        cspline_hermite = 31 // cubic hermite splines (local, only C^1)
    };

    // boundary condition type for the spline end-points
    enum bd_type { first_deriv = 1, second_deriv = 2, not_a_knot = 3 };

  protected:
    std::vector<float> m_x, m_y; // x,y coordinates of points
    // interpolation parameters
    // f(x) = a_i + b_i*(x-x_i) + c_i*(x-x_i)^2 + d_i*(x-x_i)^3
    // where a_i = y_i, or else it won't go through grid points
    std::vector<float> m_b, m_c, m_d; // spline coefficients
    float m_c0;                       // for left extrapolation
    spline_type m_type;
    bd_type m_left, m_right;
    float m_left_value, m_right_value;
    bool m_made_monotonic;
    void set_coeffs_from_b(); // calculate c_i, d_i from b_i
    // void set_coeffs_from_b_polyfrank();           // calculate c_i, d_i from b_i
    // return the closest idx so that m_x[idx] <= x (return 0 if x<m_x[0])
    size_t find_closest(float x) const {
        std::vector<float>::const_iterator it;
        it = std::upper_bound(m_x.begin(), m_x.end(), x);    // *it > x
        size_t idx = std::max(int(it - m_x.begin()) - 1, 0); // m_x[idx] <= x
        return idx;
    }
    // size_t find_closest_polyfrank(float x) const; // closest idx so that m_x[idx]<=x

  public:
    // default constructor: set boundary condition to be zero curvature
    // at both ends, i.e. natural splines
    spline()
        : m_type(cspline), m_left(second_deriv), m_right(second_deriv), m_left_value(0.0f), m_right_value(0.0f),
          m_made_monotonic(false) {
        ;
    }
    spline(const std::vector<float> &X, const std::vector<float> &Y, spline_type type = cspline,
           bool make_monotonic = false, bd_type left = second_deriv, float left_value = 0.0f,
           bd_type right = second_deriv, float right_value = 0.0f)
        : m_type(type), m_left(left), m_right(right), m_left_value(left_value), m_right_value(right_value),
          m_made_monotonic(false) // false correct here: make_monotonic() sets it
    {
        int n = (int)X.size();
        m_b.resize(n);
        m_c.resize(n);
        m_d.resize(n);
        this->set_points(X, Y, m_type);
        if (make_monotonic) {
            this->make_monotonic();
        }
    }

    // modify boundary conditions: if called it must be before set_points()
    void set_boundary(bd_type left, float left_value, bd_type right, float right_value);

    // set all data points (cubic_spline=false means linear interpolation)
    void set_points(const std::vector<float> &x, const std::vector<float> &y, spline_type type = cspline);
    // void set_points_polyfrank(const std::vector<float> &x, const std::vector<float> &y, spline_type type = cspline);

    // adjust coefficients so that the spline becomes piecewise monotonic
    // where possible
    //   this is done by adjusting slopes at grid points by a non-negative
    //   factor and this will break C^2
    //   this can also break boundary conditions if adjustments need to
    //   be made at the boundary points
    // returns false if no adjustments have been made, true otherwise
    bool make_monotonic();

    // evaluates the spline at point x
    float operator()(float x) const;
    // float getValueWithoutExtrapolation(float x) const;
    float deriv(int order, float x) const;

    // solves for all x so that: spline(x) = y
    std::vector<float> solve(float y, bool ignore_extrapolation = true) const;

    // returns the input data points
    std::vector<float> get_x() const { return m_x; }
    std::vector<float> get_y() const { return m_y; }
    float get_x_min() const {
        assert(!m_x.empty());
        return m_x.front();
    }
    float get_x_max() const {
        assert(!m_x.empty());
        return m_x.back();
    }

    float getValueWithoutExtrapolation(float &x) const {
        // polynomial evaluation using Horner's scheme
        // TODO: consider more numerically accurate algorithms, e.g.:
        //   - Clenshaw
        //   - Even-Odd method by A.C.R. Newbery
        //   - Compensated Horner Scheme
        // size_t n = m_x.size();

        size_t idx = 0;
        while (m_x[idx] < x) {
            idx++;
        }
        idx--;

        float h = x - m_x[idx];
        return ((m_d[idx] * h + m_c[idx]) * h + m_b[idx]) * h + m_y[idx];
    }
    // return the closest idx so that m_x[idx] <= x (return 0 if x<m_x[0])
    size_t find_closest_polyfrank(float x) const {
        // std::vector<float>::const_iterator it;
        // it = std::upper_bound(m_x.begin(), m_x.end(), x); // *it > x
        // size_t idx = int(it - m_x.begin()) - 1;           // m_x[idx] <= x
        // return idx;
        size_t index = 0;
        while (m_x[index] < x) {
            index++;
        }
        return index - 1;
    }
    inline void set_coeffs_from_b_polyfrank() {

        size_t n = m_b.size();

        for (size_t i = 0; i < n - 1; i++) {
            const float h = m_x[i + 1] - m_x[i];
            // from continuity and differentiability condition
            m_c[i] = (3.0f * (m_y[i + 1] - m_y[i]) / h - (2.0f * m_b[i] + m_b[i + 1])) / h;
            // from differentiability condition
            m_d[i] = ((m_b[i + 1] - m_b[i]) / (3.0f * h) - 2.0f / 3.0f * m_c[i]) / h;
        }

        // for left extrapolation coefficients
        m_c0 = m_c[0];
    }
    inline void set_points_polyfrank(const std::vector<float> &x, const std::vector<float> &y) {

        m_made_monotonic = false;
        m_x = x;
        m_y = y;
        int n = (int)x.size();

        // set b to match 1st order derivative finite difference
        for (int i = 1; i < n - 1; i++) {
            const float h = m_x[i + 1] - m_x[i];
            const float hl = m_x[i] - m_x[i - 1];
            m_b[i] =
                -h / (hl * (hl + h)) * m_y[i - 1] + (h - hl) / (hl * h) * m_y[i] + hl / (h * (hl + h)) * m_y[i + 1];
        }
        {
            const float h = m_x[1] - m_x[0];
            m_b[0] = 0.5f * (-m_b[1] - 0.5f * m_left_value * h + 3.0f * (m_y[1] - m_y[0]) / h);
        }
        {
            const float h = m_x[n - 1] - m_x[n - 2];
            m_b[n - 1] = 0.5f * (-m_b[n - 2] + 0.5f * m_right_value * h + 3.0f * (m_y[n - 1] - m_y[n - 2]) / h);
            m_c[n - 1] = 0.5f * m_right_value;

            m_d[n - 1] = 0.0f;
        }

        // parameters c and d are determined by continuity and differentiability
        set_coeffs_from_b_polyfrank();

        // for left extrapolation coefficients
        m_c0 = m_c[0];
    }

#ifdef HAVE_SSTREAM
    // spline info string, i.e. spline type, boundary conditions etc.
    std::string info() const;
#endif // HAVE_SSTREAM
};

namespace internal {

// band matrix solver
class band_matrix {
  private:
    std::vector<std::vector<float>> m_upper; // upper band
    std::vector<std::vector<float>> m_lower; // lower band
  public:
    band_matrix(){};                        // constructor
    band_matrix(int dim, int n_u, int n_l); // constructor
    ~band_matrix(){};                       // destructor
    void resize(int dim, int n_u, int n_l); // init with dim,n_u,n_l
    int dim() const;                        // matrix dimension
    int num_upper() const { return (int)m_upper.size() - 1; }
    int num_lower() const { return (int)m_lower.size() - 1; }
    // access operator
    float &operator()(int i, int j);      // write
    float operator()(int i, int j) const; // read
    // we can store an additional diagonal (in m_lower)
    float &saved_diag(int i);
    float saved_diag(int i) const;
    void lu_decompose();
    std::vector<float> r_solve(const std::vector<float> &b) const;
    std::vector<float> l_solve(const std::vector<float> &b) const;
    std::vector<float> lu_solve(const std::vector<float> &b, bool is_lu_decomposed = false);
};

float get_eps();

std::vector<float> solve_cubic(float a, float b, float c, float d, int newton_iter = 0);

} // namespace internal

} // namespace tk

#pragma GCC diagnostic pop
