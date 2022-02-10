/// \file Helpers.cpp
///
/// \brief Code for helper functions.

// MIT License
//
// Copyright (c) 2022 Ian Parberry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <algorithm>

/// Compute the cubic spline of a parameter \f$t\f$, that is, \f$3t^2 - 2t^3\f$.
/// \param t Parameter.
/// \return Cubic spline of parameter.

float spline3(float t){
  return t*t*(3.0f - 2.0f*t);
} //spline3

/// Compute the quintic spline of a parameter \f$t\f$, that is, 
/// \f$10t^3 - 15t^4 + 6t^5\f$.
/// \param t Parameter.
/// \return Quintic spline of parameter.

float spline5(float t){
  return t*t*t*(10.0f + 3.0f*t*(2.0f*t - 5.0f));
} //spline5

/// Linear interpolation between two values.
/// \param t Interpolation fraction, assumed to be in \f$[0,1]\f$.
/// \param a Lower value.
/// \param b Upper value.
/// \return A \f$\mathsf{t}\f$ fraction of the way between \f$\mathsf{a}\f$
/// and \f$\mathsf{b}\f$, that is, \f$\mathsf{(1-t)a + tb}\f$.

float lerp(float t, float a, float b){
  return a + t*(b - a);
} //lerp

/// Clamp between two bounds.
/// \param a Lower bound, assumed to be less than \f$\mathsf{b}\f$.
/// \param x Value to be clamped.
/// \param b Upper bound, assumed to be greater than \f$\mathsf{a}\f$.
/// \return Closest value to \f$\mathsf{x}\f$ between the lower and upper,
/// bounds (inclusive), that is, \f$\mathsf{\max(a, \min(x, b))}\f$.

float clamp(float a, float x, float b){
  return std::max(a, std::min(x, b));
} //clamp