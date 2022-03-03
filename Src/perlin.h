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

#include <windows.h>
#include <windowsx.h>
#include <random>

#include "Defines.h"

/// \brief 2D Perlin and Value noise generator.

class CPerlinNoise2D{
  private:
    eHash m_eHash = eHash::Permutation; ///< Hash function type.
    eSpline m_eSpline = eSpline::Cubic; ///< Spline function type.
    eDistribution m_eDistribution = eDistribution::Uniform; ///< Uniform distribution..

    size_t* m_nPerm = nullptr; ///< Random permutation, used for hash function.
    float* m_fTable = nullptr; ///< Table of gradients or values.
    
    std::default_random_engine m_stdRandom; ///< PRNG.
    UINT m_nSeed = 0; ///< PRNG seed.

    const size_t m_nDefTableSize = 256; ///< Default table size.
    const size_t m_nMinTableSize = 16; ///< Min table size.
    const size_t m_nMaxTableSize = 1024; ///< Max table size.

    size_t m_nSize = m_nDefTableSize; ///< Table size, must be a power of 2.
    size_t m_nMask = m_nDefTableSize - 1; ///< Mask for values less than `m_nSize`.
    
    inline const size_t pair(size_t, size_t) const; ///< Perlin pairing function.
    inline const size_t pairstd(size_t, size_t) const; ///< Std pairing function.
    
    inline const size_t hash(size_t) const; ///< Perlin hash function.
    inline const size_t hashstd(size_t) const; ///< std::hash function.
    inline const size_t hash2(size_t, size_t) const; ///< Hash function.

    void HashCorners(size_t, size_t, size_t[4]) const; ///< Hash grid corners.
    
    void RandomizeTableUniform(); ///< Randomize table using uniform distribution.
    void RandomizeTableCos(); ///< Randomize table using cosine.
    void RandomizeTableNormal(); ///< Randomize table using normal distribution.
    void RandomizeTableExp(); ///< Randomize table using exponential distribution.
    void RandomizeTableMaximal(); ///< Randomize table using large magnitude values.

    void RandomizeTableMidpoint(size_t, size_t, float); ///< Midpoint displacement.
    void RandomizeTableMidpoint(); ///< Randomize table using midpoint displacement.

    inline const float spline(float) const; ///< Spline curve.
    inline const float z(size_t, float, float, eNoise) const; ///< Apply gradients.
    const float Lerp(float, float, float, size_t*, eNoise) const; ///< Linear interpolation.
    const float noise(float, float, eNoise) const; ///< Perlin noise.

    void RandomizePermutation(); ///< Randomize permutation.
    void Initialize(); ///< Initialize.

  public:
    CPerlinNoise2D(); ///< Constructor.
    ~CPerlinNoise2D(); ///<Destructor.
    
    void RandomizeTable(eDistribution); ///< Randomize table from distribution.
    void Randomize(); ///< Randomize PRNG.

    bool DoubleTableSize(); ///< Double table size.
    bool HalveTableSize(); ///< Halve table size.
    bool DefaultTableSize(); ///< Set table size to default.
    
    void SetSpline(eSpline); ///< Set spline function.
    void SetHash(eHash); ///< Set hash function.
    
    const float generate(float, float, eNoise, size_t, float=0.5f, float=2.0f) const; ///< Perlin noise.
    
    const size_t GetTableSize() const; ///< Get table size.
    const size_t GetMinTableSize() const; ///< Get minimum table size.
    const size_t GetMaxTableSize() const; ///< Get maximum table size.
    const size_t GetDefTableSize() const; ///< Get default table size.
}; //CPerlinNoise2D

#endif //__PERLIN_H__
