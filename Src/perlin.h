/// \file perlin.h
///
/// \brief Interface for the Perlin and Value noise generators.

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

#ifndef __PERLIN_H__
#define __PERLIN_H__

#include "Defines.h"

/// \brief 2D Perlin and Value noise generator.

class CPerlinNoise2D{
  private:
    size_t m_nSize = 0; ///< Table size, must be a power of 2.
    size_t m_nMask = 0; ///< Mask for values less than `m_nSize`.

    size_t* m_nPerm = nullptr; ///< Random permutation, used for hash function.
    float* m_fTable = nullptr; ///< Table of gradients or values.
    
    eSpline m_eSpline = eSpline::Cubic; ///< Spline function type.
    
    inline const size_t pair(size_t, size_t) const; ///< Perlin pairing function.
    inline const size_t hash(size_t) const; ///< Perlin hash function.

    inline const float grad(size_t, float, float) const; ///< Apply gradients.
    const float noise(float, float, eNoise) const; ///< Perlin noise.

  public:
    CPerlinNoise2D(size_t); ///< Constructor.
    ~CPerlinNoise2D(); ///<Destructor.
    
    void Initialize(eDistribution); ///< Initialize table from distribution.
    void SetSpline(eSpline); ///< Set spline function.

    void Randomize(); ///< Randomize permutation.
    
    const float generate(float, float, eNoise, size_t, float=0.5f, float=2.0f) const; ///< Perlin noise.
}; //CPerlinNoise2D

#endif //__PERLIN_H__
