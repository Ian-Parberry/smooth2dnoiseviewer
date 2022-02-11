/// \file perlin.cpp
///
/// \brief Code for the Perlin and Value noise generators.

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

#include <stdlib.h>
#include <algorithm>
#include <random>

#include "perlin.h"
#include "Helpers.h"

////////////////////////////////////////////////////////////////////////////////
// CPerlinNoise2D functions.

#pragma region Constructor and destructor

/// The constructor initializes the size, which must be a power of 2, and the 
/// mask, which must be a consecutive sequence of 1 bits that is one less than
/// the size. It also creates space for the permutation and initializes it to
/// a pseudorandom permutation and creates space for the gradient/height array
/// and fills it with pseudorandom values from a uniform dietribution.
/// \param n Number of consecutive 1 bits in the mask.

CPerlinNoise2D::CPerlinNoise2D(size_t n):
  m_nSize((size_t)round(pow(2.0f, (float)n))), //a power of 2
  m_nMask(m_nSize - 1), //mask of consecutive 1s
  m_nPerm(new size_t[m_nSize]), //permutation
  m_fTable(new float[m_nSize]) //gradients or heights
{ 
  Initialize(eDistribution::Uniform);
  Randomize();
} //constructor

/// The destructor deletes the permutation and gradient/height array created in
/// the constructor.

CPerlinNoise2D::~CPerlinNoise2D(){
  delete [] m_fTable;
  delete [] m_nPerm;
} //destructor

#pragma endregion Constructor and destructor

#pragma region General CPerlinNoise2D functions

/// Set the permutation in `m_nPerm` using the standard random
/// permutation algorithm so that each permutation is equally likely.

void CPerlinNoise2D::Randomize(){
  for(size_t i=0; i<m_nSize; i++) //identity permutation
    m_nPerm[i] = i; 

  for(size_t i=0; i<m_nSize; i++) //randomize
    std::swap(m_nPerm[i], m_nPerm[i + rand()%(m_nSize - i)]);
} //Randomize

/// Initialize the table to pseudorandom gradients in \f$[-1, 1]\f$ according 
/// to some probability distribution. The pseudorandom number generators are
/// unseeded, which means that the table contents are the same each time this
/// function is called with the same parameter.
/// \param d Probability distribution enumerated type.

void CPerlinNoise2D::Initialize(eDistribution d){
  switch(d){
    case eDistribution::Uniform:
      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = (2.0f*i)/m_nSize - 1.0f;
    break;

    case eDistribution::Cosine: 
      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = cosf(((PI*i)/m_nSize));
    break;
    
    case eDistribution::Normal: {   
      std::default_random_engine g;
      std::normal_distribution<float> d(500.0f, 200.0f);

      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = 2.0f*clamp(0.0f, d(g)/1000.0f, 1.0f) - 1.0f;
      } //case
    break;

    case eDistribution::Exponential: {
      std::default_random_engine g;
      std::exponential_distribution<float> d(3.5f);

      const size_t half = m_nSize/2;

      for(size_t i=0; i<half; i++) //positive values
        m_fTable[i] = clamp(0.0f, d(g), 1.0f);

      for(size_t i=half; i<m_nSize; i++) //negative values
        m_fTable[i] = -clamp(0.0f, d(g), 1.0f);
    } //case
    break;
  } //switch
} //Initialize

/// Set the spline function type.
/// \param d Spline function enumerated type.

void CPerlinNoise2D::SetSpline(eSpline d){
  m_eSpline = d;
} //SetSpline

/// Apply gradient to a point.
/// \param h Hash value for gradient.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \return The gradient applied to the point.

inline const float CPerlinNoise2D::grad(size_t h, float x, float y) const{
  return x*m_fTable[h] + y*m_fTable[m_nPerm[h]];
} //grad

/// Perlin's pairing function, which combines two unsigned integers into one.
/// \param x First unsigned integer.
/// \param y Second unsigned integer.
/// \return An unsigned integer that depends upin x and y.

inline const size_t CPerlinNoise2D::pair(size_t x, size_t y) const{
  return hash(x) + y;
} //pair

/// Perlin's hash function, which uses a random permutation. Note that this
/// means that it repeats with a period of m_nSize.
/// \param x An unsigned integer.
/// \return A hashed value of x in the range [0, m_nSize - 1].

inline const size_t CPerlinNoise2D::hash(size_t x) const{
  return m_nPerm[x & m_nMask];
} //hash

/// Compute a single octave of Perlin noise at a 2D point.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \param t Noise type.
/// \return Perlin noise value in [-1, 1] at the given point.

const float CPerlinNoise2D::noise(float x, float y, eNoise t) const{
  const size_t nX = (size_t)floorf(x); //integer part of x
  const size_t nY = (size_t)floorf(y); //integer part of y

  const float fX = x - floorf(x); //fractional part of x
  const float fY = y - floorf(y); //fractional part of y
 
  float u, v; //for spline function along x-axis

  switch(m_eSpline){
    case eSpline::None:    u = fX;          v = fY; break;
    case eSpline::Cubic:   u = spline3(fX); v = spline3(fY); break;
    case eSpline::Quintic: u = spline5(fX); v = spline5(fY);break;
  } //switch
  
  //hash value at corners of surrounding square with integer coordinates

  const size_t AA = hash(pair(nX,     nY)); 
  const size_t BA = hash(pair(nX + 1, nY));
  const size_t AB = hash(pair(nX,     nY + 1)); 
  const size_t BB = hash(pair(nX + 1, nY + 1));

  //now lerp along the top and bottom in the x direction

  float a=0, b=0; //for lerped values

  switch(t){ //depending on whether Perlin noise or Value noise
    case eNoise::Perlin:
      a = lerp(u, grad(AA, fX, fY),     grad(BA, fX - 1, fY));
      b = lerp(u, grad(AB, fX, fY - 1), grad(BB, fX - 1, fY - 1));
     break;
      
    case eNoise::Value:
      a = lerp(u, m_fTable[AA], m_fTable[BA]);
      b = lerp(u, m_fTable[AB], m_fTable[BB]);
    break;
  } //switch

  //now lerp in the other direction

  return lerp(v, a, b);
} //noise
  
/// Uses multiple octaves of Perlin noise to compute an effect similar to
/// turbulence at a single point.
/// \param x X-coordinate of a 2D point.
/// \param y Y-coordinate of a 2D point.
/// \param t Noise type.
/// \param alpha Lacunarity.
/// \param beta Persistence.
/// \param n Number of octaves.
/// \return Turbulence value in [-1, 1] at point (x, y).

const float CPerlinNoise2D::generate(float x, float y, eNoise t, float alpha, 
  float beta, size_t n) const
{
  float sum = 0.0f; //for sum of octaves
  float scale = 1.0f; //for octave scale

  for(size_t i=0; i<n; i++){ //for each octave
    sum += scale*noise(x, y, t); //scale noise by octave scale
    scale *= alpha; //reduce octave scale
    x *= beta; 
    y *= beta;
  } //for

  //scale result by sum of geometric progression
  const float result = (1 - alpha)*sum/(1 - scale); 

  //scale up for Perlin noise
  return (t == eNoise::Perlin)? SQRT2*result: result;
} //generate

#pragma endregion