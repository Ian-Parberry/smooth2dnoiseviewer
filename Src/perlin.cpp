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

///////////////////////////////////////////////////////////////////////////////
// Helper functions.

#pragma region Helper functions

float clamp(float lower, float x, float upper){
  return std::max(lower, std::min(x, upper));
} //clamp

/// \brief Cubic spline.
///
/// Compute the cubic spline of a parameter \f$t\f$, that is, \f$3t^2 - 2t^3\f$.
/// \param t Parameter.
/// \return Cubic spline of parameter.

inline float spline3(float t){
  return t*t*(3.0f - 2.0f*t);
} //spline3

/// \brief Quintic spline.
///
/// Compute the quintic spline of a parameter \f$t\f$, that is, 
/// \f$10t^3 - 15t^4 + 6t^5\f$.
/// \param t Parameter.
/// \return Quintic spline of parameter.

inline float spline5(float t){
  return t*t*t*(10.0f + 3.0f*t*(2.0f*t - 5.0f));
} //spline5

/// \brief Linear interpolation.
///
/// Linear interpolation between two values.
/// \param t Interpolation fraction.
/// \param a Lower bound.
/// \param b Upper bound.
/// \return \f$(1-t)a + tb\f$.

inline float lerp(float t, float a, float b){
  return a + t*(b - a);
} //lerp

#pragma endregion

////////////////////////////////////////////////////////////////////////////////
// CPerlinNoise2D functions.

#pragma region CPerlinNoise2D functions

/// The constructor initializes the size, which must be a power of 2, and the 
/// mask, which must be a consecutive sequence of 1 bits that is one less than
/// the size. It also creates space for the permutation and initializes it to
/// a pseudorandom permutation.
/// \param n Number of consecutive 1 bits in the mask.

CPerlinNoise2D::CPerlinNoise2D(size_t n){
  m_nSize = (size_t)round(pow(2.0f, (float)n)); //a power of 2
  m_nMask = m_nSize - 1; //mask of consecutive 1s
  m_nPerm = new size_t[m_nSize]; //permutation
  m_fGradient = new float[m_nSize]; //gradients
  SetDistribution(eDistribution::Uniform);

  Randomize();
} //constructor

/// The destructor deletes the permutation and gradient array created in the
/// constructor.

CPerlinNoise2D::~CPerlinNoise2D(){
  delete [] m_fGradient;
  delete [] m_nPerm;
} //destructor

/// Initialize the permutation in `m_nPerm` using the standard random
/// permutation algorithm so that each permutation is equally likely.

void CPerlinNoise2D::Randomize(){
  for(size_t i=0; i<m_nSize; i++) //identity permutation
    m_nPerm[i] = i; 

  for(size_t i=0; i<m_nSize; i++) //randomize
    std::swap(m_nPerm[i], m_nPerm[i + rand()%(m_nSize - i)]);
} //Randomize

/// Initialize the gradient array to pseudorandom gradients in [-1, 1]
/// according to some probability distribution.
/// \param d Probability distribution enumerated type.

void CPerlinNoise2D::SetDistribution(eDistribution d){
  switch(d){
    case eDistribution::Uniform:
      for(size_t i=0; i<m_nSize; i++)
        m_fGradient[i] = (2.0f*i)/m_nSize - 1.0f;
    break;

    case eDistribution::Cosine: 
      for(size_t i=0; i<m_nSize; i++)
        m_fGradient[i] = cosf(((PI*i)/m_nSize));
    break;
    
    case eDistribution::Normal: {   
      std::default_random_engine g;
      std::normal_distribution<float> d(500.0f, 200.0f);

      for(size_t i=0; i<m_nSize; i++)
        m_fGradient[i] = 2.0f*clamp(0.0f, (d(g))/1000.0f, 1.0f) - 1.0f;
      } //case
    break;

    case eDistribution::Exponential: {
      std::default_random_engine g;
      std::exponential_distribution<float> d(3.5f);

      const size_t half = m_nSize/2;

      for(size_t i=0; i<half; i++)
        m_fGradient[i] = clamp(0.0f, d(g), 1.0f);

      for(size_t i=half; i<m_nSize; i++)
        m_fGradient[i] = -clamp(0.0f, d(g), 1.0f);
    } //case
    break;
  } //switch
} //SetDistribution

/// Apply gradient to a point.
/// \param h Hash value.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \return The gradient applied to the point.

const float CPerlinNoise2D::grad(size_t h, float x, float y) const{
  return x*m_fGradient[h] + y*m_fGradient[m_nPerm[h]];
} //grad

/// Compute a single octave of Perlin noise at a 2D point.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \param t Noise type.
/// \return Perlin noise value in [-1, 1] at the given point.

const float CPerlinNoise2D::noise(float x, float y, eNoise t) const{
  const size_t nX = (size_t)floorf(x) & m_nMask;
  const size_t nY = (size_t)floorf(y) & m_nMask;

  const float fX = x - floorf(x);
  const float fY = y - floorf(y);
 
  const float u = spline3(fX);
  const float v = spline3(fY);
  
  const size_t AA = m_nPerm[(m_nPerm[nX] + nY) & m_nMask]; 
  const size_t BA = m_nPerm[(m_nPerm[(nX + 1) & m_nMask] + nY) & m_nMask];
  const size_t AB = m_nPerm[(m_nPerm[nX] + nY + 1) & m_nMask]; 
  const size_t BB = m_nPerm[(m_nPerm[(nX + 1) & m_nMask] + nY + 1) & m_nMask];

  float a=0, b=0;

  switch(t){
    case eNoise::Perlin:
      a = lerp(u, grad(AA, fX, fY), grad(BA, fX - 1, fY));
      b = lerp(u, grad(AB, fX, fY - 1), grad(BB, fX - 1, fY - 1));
     break;
      
    case eNoise::Value:
      a = lerp(u, m_fGradient[AA], m_fGradient[BA]);
      b = lerp(u, m_fGradient[AB], m_fGradient[BB]);
    break;
  } //switch


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
  float sum = 0.0f, scale = 1.0f;

  for(size_t i=0; i<n; i++){
    sum += scale*noise(x, y, t);
    scale *= alpha;
    x *= beta; 
    y *= beta;
  } //for

  return (t == eNoise::Perlin? SQRT2: 1.0f)*(1.0f - alpha)*sum/(1.0f - scale);
} //generate

#pragma endregion