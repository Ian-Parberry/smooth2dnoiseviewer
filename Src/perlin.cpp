/// \file perlin.cpp

#include <stdlib.h>
#define _USE_MATH_DEFINES ///< Enable use of constant M_SQRT2 in math.h
#include <math.h>
#include <algorithm>

#include "perlin.h"

#define B 0x100
#define BM 0xff

static int p[B + B + 2];
static float g[B + B + 2][2];

const float SQRT2 = (float)M_SQRT2;

inline float spline3(float t){
  return t*t*(3.0f - 2.0f*t);
} //spline3

inline float spline5(float t){
  return t*t*t*(10.0f + 3.0f*t*(2.0f*t - 5.0f));
} //spline5

inline float lerp(float t, float a, float b){
  return a + t*(b - a);
} //lerp

inline void setup(float v, int& b0, int& b1, float& r0, float& r1){
  b0 = ((int)v) & BM;
  b1 = (b0 + 1) & BM;
  r0 = v - (int)v;
  r1 = r0 - 1.0f;
} //setup

float at(float q[2], float rx, float ry){
  return rx*q[0] + ry*q[1];
} //at

void normalize2(float v[2]){
  const float s = sqrtf(v[0]*v[0] + v[1]*v[1]);
  v[0] /= s; v[1] /= s;
} //normalize2

void initPerlin(){
  for(int i=0; i<B ; i++){
    p[i] = i;
    g[i][0] = (float)(rand()%(B+B)-B)/B; 
    g[i][1] = (float)(rand()%(B+B)-B)/B; 
    normalize2(g[i]);
  } //for

  for(int i=B-1; i>=0; i--)
    std::swap(p[i], p[rand()%(i+1)]);

  for(int i=0; i<B+2; i++){
    p[B + i] = p[i];
    g[B + i][0] = g[i][0];
    g[B + i][1] = g[i][1];
  } //for
} //initPerlin

float noise2(float vec[2]){
  int bx0, bx1, by0, by1;
  float rx0, rx1, ry0, ry1;

  setup(vec[0], bx0, bx1, rx0, rx1);
  setup(vec[1], by0, by1, ry0, ry1);
      
  const int i = p[bx0], j = p[bx1];
  const int b00 = p[i+by0], b10 = p[j+by0], b01 = p[i+by1], b11 = p[j+by1];

  const float sx = spline3(rx0);

  float u = at(g[b00], rx0, ry0);
  float v = at(g[b10], rx1, ry0);
  const float a = lerp(sx, u, v);

  u = at(g[b01], rx0, ry1);
  v = at(g[b11], rx1, ry1);
  const float b = lerp(sx, u, v);
   
  const float sy = spline3(ry0);
  return lerp(sy, a, b);
} //noise2

float PerlinNoise2D(float x, float y, float alpha, float beta, int n){
  float sum = 0.0f, p[2], scale = 1.0f;
  p[0] = x; p[1] = y;

  for(int i=0; i<n; i++){
    sum += noise2(p)*scale;
    scale *= alpha;
    p[0] *= beta; 
    p[1] *= beta;
  } //for

  return SQRT2*(1.0f - alpha)*sum/(1.0f - scale); 
} //PerlinNoise2D

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

float ValueNoise2D(float x, float y, float alpha, float beta, int n){
  float sum = 0.0f, p[2], scale = 1.0f;
  p[0] = x; p[1] = y;

  for(int i=0; i<n; i++){
    sum += vnoise2(p)*scale;
    scale *= alpha;
    p[0] *= beta; 
    p[1] *= beta;
  } //for

  return (1.0f - alpha)*sum/(1.0f - scale); 
} //ValueNoise2D