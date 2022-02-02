/// \file perlin.cpp
///
/// \brief Code for the Perlin and Value noise generators.

#include <stdlib.h>
#define _USE_MATH_DEFINES ///< Enable use of constant M_SQRT2 in math.h
#include <math.h>
#include <algorithm>

#include "perlin.h"

const float SQRT2 = (float)M_SQRT2; ///< Square root of 2.
#define sqr(x) ((x)*(x)) ///< Squaring function.

///////////////////////////////////////////////////////////////////////////////
// Helper functions.

#pragma region Helper functions

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

/// Initialize the gradients in `m_fXGradient` and `m_fYGradient` such that 
/// the vector [m_fXGradient[i], m_fYGradient[i]] is a unit vector.

void CPerlinNoise2D::RandomizeGradients(){
  for(size_t i=0; i<512; i++){
    m_fXGradient[i] = (2.0f*rand())/RAND_MAX - 1.0f; 
    m_fYGradient[i] = (2.0f*rand())/RAND_MAX - 1.0f;
    const float s = sqrtf(sqr(m_fXGradient[i]) + sqr(m_fYGradient[i]));
    m_fXGradient[i] /= s;
    m_fYGradient[i] /= s;
  } //for
} //RandomizeGradients

/// Apply gradient to point.
/// \param h Hash value.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \return The gradient applied to the point.

float CPerlinNoise2D::grad(unsigned h, float x, float y){
  return x*m_fXGradient[h] + y*m_fYGradient[h];
} //grad

/// \brief Compute Perlin noise at a given point.
///
/// Compute a single octave of Perlin noise at a 2D point.
/// \param vec A 2D point.
/// \return Perlin noise value in [-1, 1] at the given point.

float CPerlinNoise2D::perlinnoise(float x, float y) {
  const unsigned nX = (unsigned)floorf(x) & 255;
  const unsigned nY = (unsigned)floorf(y) & 255;

  const float fX = x - floorf(x);
  const float fY = y - floorf(y);
 
  const float u = spline3(fX);
  const float v = spline3(fY);
  
  const int AA = m_nPerm[m_nPerm[nX] + nY]; 
  const int BA = m_nPerm[m_nPerm[nX + 1] + nY];
  const int AB = m_nPerm[m_nPerm[nX] + nY + 1]; 
  const int BB = m_nPerm[m_nPerm[nX + 1] + nY + 1];

  const float a = lerp(u, grad(AA, fX, fY), grad(BA, fX - 1, fY));
  const float b = lerp(u, grad(AB, fX, fY - 1), grad(BB, fX - 1, fY - 1));

  return lerp(v, a, b);
} //perlinnoise
  
/// \brief Turbulence function for Perlin noise.
///
/// Uses multiple octaves of Perlin noise to compute an effect similar to
/// turbulence at a single point.
/// \param x X-coordinate of a 2D point.
/// \param y Y-coordinate of a 2D point.
/// \param alpha Lacunarity.
/// \param beta Persistence.
/// \param n Number of octaves.
/// \return Turbulence value in [-1, 1] at point (x, y).

float CPerlinNoise2D::PerlinNoise(float x, float y, float alpha, float beta, unsigned n){
  float sum = 0.0f, scale = 1.0f;

  for(unsigned i=0; i<n; i++){
    sum += perlinnoise(x, y)*scale;
    scale *= alpha;
    x *= beta; 
    y *= beta;
  } //for

  return SQRT2*(1.0f - alpha)*sum/(1.0f - scale); 
} //PerlinNoise

/// \brief Compute Value noise at a given point.
///
/// Compute a single octave of Value noise at a 2D point.
/// \param vec A 2D point.
/// \return Value noise in [-1, 1] at a given point.

float CPerlinNoise2D::valuenoise(float x, float y) {
  const unsigned nX = (unsigned)floorf(x) & 255;
  const unsigned nY = (unsigned)floorf(y) & 255;

  const float fX = x - floorf(x);
  const float fY = y - floorf(y);
 
  const float u = spline3(fX);
  const float v = spline3(fY);
  
  const unsigned AA = m_nPerm[m_nPerm[nX] + nY]; 
  const unsigned BA = m_nPerm[m_nPerm[nX + 1] + nY];
  const unsigned AB = m_nPerm[m_nPerm[nX] + nY + 1]; 
  const unsigned BB = m_nPerm[m_nPerm[nX + 1] + nY + 1];

  const float a = lerp(u, m_fXGradient[AA], m_fXGradient[BA]);
  const float b = lerp(u, m_fXGradient[AB], m_fXGradient[BB]);

  return lerp(v, a, b);
} //valuenoise

/// \brief Turbulence function for Value noise.
///
/// Uses multiple octaves of Value noise to compute an effect similar to
/// turbulence at a single point.
/// \param x X-coordinate of a 2D point.
/// \param y Y-coordinate of a 2D point.
/// \param alpha Lacunarity.
/// \param beta persistence.
/// \param n Number of octaves.
/// \return Turbulence value in [-1, 1] at point (x, y).

float CPerlinNoise2D::ValueNoise(float x, float y, float alpha, float beta, unsigned n){
  float sum = 0.0f, scale = 1.0f;

  for(unsigned i=0; i<n; i++){
    sum += valuenoise(x, y)*scale;
    scale *= alpha;
    x *= beta; 
    y *= beta;
  } //for

  return (1.0f - alpha)*sum/(1.0f - scale); 
} //ValueNoise

#pragma endregion