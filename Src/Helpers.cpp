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
#include <sstream>

#include "Helpers.h"

/// Compute the cubic spline of a parameter \f$t\f$, that is, \f$3t^2 - 2t^3\f$.
/// \param t Parameter.
/// \return Cubic spline of parameter.

const float spline3(float t){
  return t*t*(3.0f - 2.0f*t);
} //spline3

/// Compute the quintic spline of a parameter \f$t\f$, that is, 
/// \f$10t^3 - 15t^4 + 6t^5\f$.
/// \param t Parameter.
/// \return Quintic spline of parameter.

const float spline5(float t){
  return t*t*t*(10.0f + 3.0f*t*(2.0f*t - 5.0f));
} //spline5

/// Linear interpolation between two values.
/// \param t Interpolation fraction, assumed to be in \f$[0,1]\f$.
/// \param a Lower value.
/// \param b Upper value.
/// \return A \f$\mathsf{t}\f$ fraction of the way between \f$\mathsf{a}\f$
/// and \f$\mathsf{b}\f$, that is, \f$\mathsf{(1-t)a + tb}\f$.

const float lerp(float t, float a, float b){
  return a + t*(b - a);
} //lerp

/// Clamp between two bounds.
/// \param a Lower bound, assumed to be less than \f$\mathsf{b}\f$.
/// \param x Value to be clamped.
/// \param b Upper bound, assumed to be greater than \f$\mathsf{a}\f$.
/// \return Closest value to \f$\mathsf{x}\f$ between the lower and upper,
/// bounds (inclusive), that is, \f$\mathsf{\max(a, \min(x, b))}\f$.

const float clamp(float a, float x, float b){
  return std::max(a, std::min(x, b));
} //clamp

/// Convert a floating point number into a fixed precision wide string.
/// \param x A floating point number.
/// \param n Number of digits after the decimal point.
/// \return Fixed precision string.

std::wstring to_wstring_f(float x, size_t n){
  std::wstringstream s; //convertor
  s.precision(n); //set precision
  s << std::fixed << x; //convert x
  return s.str(); //return wstring
} //to_wstring_f

/// Test whether an unsigned integer is a power of 2. This is a sneaky one. 
/// It relies pm the fact that `n` is a power of 2 iff `n` is nonzero and
/// `n & (n - 1)` is zero. To see this, note that if \f$n = 2^k\f$, then the
/// binary representation of \f$n\f$ is a one followed by \f$k\f$ zeros and
/// that of \f$n-1\f$ is a zero followed by \f$k\f$ ones. Clearly the bit-wise
/// conjunction of the two is zero. Conversely, if \f$n\f$ is not a power
/// of two, then the binary representations of \f$n\f$ and \f$n-1\f$ will share
/// one-bits in the same position so their conjunction will be non-zero.
/// \param n A number.
/// \return true if the parameter is a power of 2.

const bool isPowerOf2(size_t n){
  return n != 0 && (n & (n - 1)) == 0;
} //isPowerOf2