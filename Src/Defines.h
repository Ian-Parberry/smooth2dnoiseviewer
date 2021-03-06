/// \file Defines.h
/// \brief Useful defines, constants, and types.

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

#ifndef __DEFINES_H__
#define __DEFINES_H__

#define _USE_MATH_DEFINES ///< Enable use of constant M_PI in math.h
#include <math.h>

const float PI = (float)M_PI; ///< Pi.

/// \brief Perlin noise type.
///
/// Enumerated type for Perlin noise.

enum class eNoise{
  None, Perlin, Value
}; //eNoise

/// \brief Hash function type.
///
/// Enumerated type for hash function.

enum class eHash{
  Permutation, LinearCongruential, Std
}; //eHash

/// \brief Distribution.
///
/// Enumerated type for probability distribution.

enum class eDistribution{
  Uniform, Maximal, Cosine, Normal, Exponential, Midpoint
}; //eDistribution

/// \brief Spline type.
///
/// Enumerated type for spline functions.

enum class eSpline{
  None, Cubic, Quintic
}; //eSpline

#endif //__DEFINES_H__
