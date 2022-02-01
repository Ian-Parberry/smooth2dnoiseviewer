/// \file perlin.cpp
///
/// \brief Code for the Perlin and Value noise generators.

#include <stdlib.h>
#define _USE_MATH_DEFINES ///< Enable use of constant M_SQRT2 in math.h
#include <math.h>
#include <algorithm>

#include "perlin.h"

#define B 0x100 ///< A power of 2.
#define BM 0xff ///< Mask.

static int p[B + B + 2]; ///< Permutation.
static float g[B + B + 2][2];///< Gradients.

const float SQRT2 = (float)M_SQRT2; ///< Square root of 2.

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

/// \brief Perlin's setup function.
///
/// Separate the integer and fractional parts of a floating point value.
/// \param v A floating point value.
/// \param b0 The integer part of v masked by BM.
/// \param b1 The integer part of v plus 1, masked by BM.
/// \param r0 The fractional part of v.
/// \param r1 The fractional part of v minus one.

inline void setup(float v, int& b0, int& b1, float& r0, float& r1){
  b0 = ((int)v) & BM;
  b1 = (b0 + 1) & BM;
  r0 = v - (int)v;
  r1 = r0 - 1.0f;
} //setup

/// \brief Initialize Perlin noise.
///
/// Initialize the gradient table and the permutation table for Perlin noise.

void initPerlin(){
  for(int i=0; i<B ; i++){
    p[i] = i;
    g[i][0] = (float)(rand()%(B+B)-B)/B; 
    g[i][1] = (float)(rand()%(B+B)-B)/B;

    const float s = sqrtf(g[i][0]*g[i][0] + g[i][1]*g[i][1]);
    g[i][0] /= s; g[i][1] /= s; //normalize gradient g[i]
  } //for

  for(int i=B-1; i>=0; i--)
    std::swap(p[i], p[rand()%(i+1)]);

  for(int i=0; i<B+2; i++){
    p[B + i] = p[i];
    g[B + i][0] = g[i][0];
    g[B + i][1] = g[i][1];
  } //for
} //initPerlin

/// \brief Compute Perlin noise at a given point.
///
/// Compute a single octave of Perlin noise at a 2D point.
/// \param vec A 2D point.
/// \return Perlin noise value in [-1, 1] at a given point.

float noise2(float vec[2]){
  int bx0, bx1, by0, by1;
  float rx0, rx1, ry0, ry1;

  setup(vec[0], bx0, bx1, rx0, rx1);
  setup(vec[1], by0, by1, ry0, ry1);
      
  const int i = p[bx0], j = p[bx1];
  const int b00 = p[i+by0], b10 = p[j+by0], b01 = p[i+by1], b11 = p[j+by1];

  const float sx = spline3(rx0);

  float u = g[b00][0]*rx0 + g[b00][1]*ry0;
  float v = g[b10][0]*rx1 + g[b10][1]*ry0;
  const float a = lerp(sx, u, v);

  u = g[b01][0]*rx0 + g[b01][1]*ry1;
  v = g[b11][0]*rx1 + g[b11][1]*ry1;
  const float b = lerp(sx, u, v);
   
  const float sy = spline3(ry0);
  return lerp(sy, a, b);
} //noise2

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

float PerlinNoise2D(float x, float y, float alpha, float beta, unsigned n){
  float sum = 0.0f, p[2], scale = 1.0f;
  p[0] = x; p[1] = y;

  for(unsigned i=0; i<n; i++){
    sum += noise2(p)*scale;
    scale *= alpha;
    p[0] *= beta; 
    p[1] *= beta;
  } //for

  return SQRT2*(1.0f - alpha)*sum/(1.0f - scale); 
} //PerlinNoise2D

/// \brief Compute Value noise at a given point.
///
/// Compute a single octave of Value noise at a 2D point.
/// \param vec A 2D point.
/// \return Value noise in [-1, 1] at a given point.

float vnoise2(float vec[2]){
  int bx0, bx1, by0, by1;
  float rx0, rx1, ry0, ry1;

  setup(vec[0], bx0, bx1, rx0, rx1);
  setup(vec[1], by0, by1, ry0, ry1);  

  const int i = p[bx0], j = p[bx1];
  const int b00 = p[i+by0], b10 = p[j+by0], b01 = p[i+by1], b11 = p[j+by1];

  const float sx = spline3(rx0);
  const float a = lerp(sx, g[b00][0], g[b10][0]);
  const float b = lerp(sx, g[b01][0], g[b11][0]);
   
  const float sy = spline3(ry0);
  return lerp(sy, a, b);
} //vnoise2

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

float ValueNoise2D(float x, float y, float alpha, float beta, unsigned n){
  float sum = 0.0f, p[2], scale = 1.0f;
  p[0] = x; p[1] = y;

  for(unsigned i=0; i<n; i++){
    sum += vnoise2(p)*scale;
    scale *= alpha;
    p[0] *= beta; 
    p[1] *= beta;
  } //for

  return (1.0f - alpha)*sum/(1.0f - scale); 
} //ValueNoise2D