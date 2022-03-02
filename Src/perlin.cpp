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
#include <functional>

#include "perlin.h"
#include "Helpers.h"
#include "Includes.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor and destructor.

#pragma region Constructor and destructor

/// The constructor initializes the size of the permutation and the table, 
/// which must be a power of 2, and  the mask, which must be a consecutive
/// sequence of 1 bits that is one less than the size. The mask will be used to
/// ensure that all indices into the permutation and the table are in range,
/// Space is created for the permutation, and for the table, which will be used
/// for a gradient table in Perlin noise and a value table in Value noise.
/// The permutation is initialized to a pseudo-random permutation from a
/// uniform distribution. The table is filled with pseudo-random values chosen
/// uniformly from \f$[-1,1]\f$.
/// \param n Number of consecutive 1 bits in the mask, log base 2 of the size.

CPerlinNoise2D::CPerlinNoise2D(size_t n):
  m_nSize((size_t)round(pow(2.0f, (float)n))), //size must be a power of 2
  m_nMask(m_nSize - 1), //mask of n consecutive 1s
  m_nPerm(new size_t[m_nSize]), //permutation
  m_fTable(new float[m_nSize]) //gradients or height values
{  
  RandomizeTable(eDistribution::Uniform); //randomize gradient/value table
  RandomizePermutation(); //randomize permutation
} //constructor

/// The destructor deletes the permutation and table created in
/// the constructor.

CPerlinNoise2D::~CPerlinNoise2D(){
  delete [] m_fTable;
  delete [] m_nPerm;
} //destructor

#pragma endregion Constructor and destructor

////////////////////////////////////////////////////////////////////////////////
// Functions that change noise settings.

#pragma region Functions that change noise settings

/// Set the permutation in `m_nPerm` to a pseudo-random permutation with
/// each permutation equally likely. The standard algorithm is used, with the
/// C standard library function `rand()` used as a source of randomness.

void CPerlinNoise2D::RandomizePermutation(){
  for(size_t i=0; i<m_nSize; i++) //identity permutation
    m_nPerm[i] = i; 

  for(size_t i=0; i<m_nSize; i++) //randomize
    std::swap(m_nPerm[i], m_nPerm[i + rand()%(m_nSize - i)]);
} //RandomizePermutation

/// Initialize a chunk of the gradient/value table `m_fTable` using midpoint
/// displacement. Given \f$\mathsf{i}\f$ and \f$\mathsf{j}\f$ such that
/// \f$\mathsf{j} > \mathsf{i}+1\f$ and
/// \f$0 \leq \mathsf{i}, \mathsf{j} \leq\f$ `m_nSize`, where
/// \f$\mathsf{j} - \mathsf{i}\f$ is a power of 2, this function assumes that
/// `m_fTable`\f$[\mathsf{i}]\f$ and  `m_fTable`\f$[\mathsf{j}]\f$ have
/// been set, and fills in the entries `m_fTable`\f$[\mathsf{i} + 1]\f$
/// through `m_fTable`\f$[\mathsf{j}-1]\f$ recursively using  midpoint
/// displacement. That is, it sets the middle entry to the average of the
/// first and last entry offset by a pseudo-random value in \f$[-1,1]\f$
/// times a value called the _lacunarity_. The function then halves the
/// lacunarity and calls itself recursively on the top and bottom halves of
/// the table chunk. The source of randomness is `std::default_random_engine`
/// with `std::uniform_real_distribution<float>(-1.0f, 1.0f)`.
/// \param i Lower index.
/// \param j Upper index.
/// \param alpha Lacunarity.

void CPerlinNoise2D::RandomizeTableMidpoint(size_t i, size_t j, float alpha){
  std::uniform_real_distribution<float> d(-1.0f, 1.0f);

  if(j > i + 1){ //base of recursion is "do nothing"
    const size_t mid = (i + j)/2; //mid point

    const float fMean = (m_fTable[i] + m_fTable[j - 1])/2.0f; //average of ends
    const float fRand = alpha*d(m_stdRandom); //random offset
    m_fTable[mid] = clamp(-1.0f, fMean + fRand, 1.0f); //mid point is average plus offset
    alpha *= 0.5f; //increase lacunarity

    RandomizeTableMidpoint(i, mid, alpha); //recurse on first half
    RandomizeTableMidpoint(mid, j, alpha); //recurse on second half
  } //if
} //RandomizeTableMidpoint

/// Fill the gradient/value table `m_fTable` using midpoint displacement.
/// This function fills in the first and last entries then calls the recursive
/// `RandomizeTableMidpoint(size_t, size_t, float)` to fill in the rest.

void CPerlinNoise2D::RandomizeTableMidpoint(){
  m_fTable[0] = 0.0f;
  m_fTable[m_nSize - 1] = 0.0f;

  RandomizeTableMidpoint(0, m_nSize, 0.5f);
} //RandomizeTableMidpoint

/// Fill the gradient/value table `m_fTable` using a uniform distribution.
/// The source of randomness is `std::default_random_engine`
/// with `std::uniform_real_distribution<float>(-1.0f, 1.0f)`.

void CPerlinNoise2D::RandomizeTableUniform(){  
  std::uniform_real_distribution<float> d(-1.0f, 1.0f);

  for(size_t i=0; i<m_nSize; i++){
    m_fTable[i] = d(m_stdRandom);
    assert(-1.0f <= m_fTable[i] && m_fTable[i] <= 1.0f);
  } //for
} //RandomizeTableUniform

/// Fill the gradient/value table `m_fTable` with large magnitude entries,
/// that is, either -1 ot +1, using a uniform distribution. The source of
/// randomness is `std::default_random_engine` with
/// `std::uniform_real_distribution<float>(-1.0f, 1.0f)`.

void CPerlinNoise2D::RandomizeTableMaximal(){  
  std::uniform_real_distribution<float> d(-1.0f, 1.0f);

  for(size_t i=0; i<m_nSize; i++){
    m_fTable[i] = (d(m_stdRandom) > 0.0f)? 1.0f: -1.0f;
    assert(-1.0f <= m_fTable[i] && m_fTable[i] <= 1.0f);
  } //for
} //RandomizeTableMax

/// Fill the gradient/value table `m_fTable` using a normal distribution.
/// The source of randomness is `std::default_random_engine`
/// with `std::normal_distribution<float>(500.0f, 200.0f)`.

void CPerlinNoise2D::RandomizeTableNormal(){  
  std::normal_distribution<float> d(500.0f, 200.0f);

  for(size_t i=0; i<m_nSize; i++){
    m_fTable[i] = 2.0f*clamp(0.0f, d(m_stdRandom)/1000.0f, 1.0f) - 1.0f;
    assert(-1.0f <= m_fTable[i] && m_fTable[i] <= 1.0f);
  } //for
} //RandomizeTableNormal

/// Fill the gradient/value table `m_fTable` using a cosine distribution.
/// The source of randomness is `std::default_random_engine`
/// with `std::uniform_real_distribution<float>(0.0f, 1.0f)`.
/// It simply multiplies the each pseudo-random number by \f$pi\f$ and
/// enters the cosine of the result into the table.

void CPerlinNoise2D::RandomizeTableCos(){  
  std::uniform_real_distribution<float> d(0.0f, 1.0f);

  for(size_t i=0; i<m_nSize; i++){
    m_fTable[i] = cosf(PI*d(m_stdRandom));
    assert(-1.0f <= m_fTable[i] && m_fTable[i] <= 1.0f);
  } //for
} //RandomizeTableCos

/// Fill the gradient/value table `m_fTable` using an exponential distribution.
/// The source of randomness is `std::default_random_engine`
/// with `std::exponential_distribution<float>(8.0f)`. It fills half of the
/// table with negative gradients and half with positive gradients.

void CPerlinNoise2D::RandomizeTableExp(){  
  std::exponential_distribution<float> d(8.0f);

  const size_t half = m_nSize/2;

  for(size_t i=0; i<half; i++) //positive values
    m_fTable[i] = clamp(0.0f, d(m_stdRandom), 1.0f);

  for(size_t i=half; i<m_nSize; i++) //negative values
    m_fTable[i] = -clamp(0.0f, d(m_stdRandom), 1.0f);
} //RandomizeTableExp

/// Set `m_fTable` to pseudo-random values in \f$[-1, 1]\f$ according 
/// to some probability distribution. The pseudo-random number generator is
/// re-seeded with a time value each time this function is called.
/// \param d Probability distribution enumerated type.

void CPerlinNoise2D::RandomizeTable(eDistribution d){ 
  m_stdRandom.seed(timeGetTime()); //reset PRNG

  switch(d){
    case eDistribution::Uniform: RandomizeTableUniform();   break;
    case eDistribution::Maximal: RandomizeTableMaximal();   break;
    case eDistribution::Cosine:  RandomizeTableCos();       break;
    case eDistribution::Normal:  RandomizeTableNormal();    break;
    case eDistribution::Exponential: RandomizeTableExp();   break;
    case eDistribution::Midpoint: RandomizeTableMidpoint(); break;
  } //switch
} //RandomizeTable

/// Set the spline function type.
/// \param d Spline function enumerated type.

void CPerlinNoise2D::SetSpline(eSpline d){
  m_eSpline = d;
} //SetSpline

/// Set the hash function type.
/// \param d Hash function enumerated type.

void CPerlinNoise2D::SetHash(eHash d){
  m_eHash = d;
} //SetHash

#pragma endregion Functions that change noise settings

////////////////////////////////////////////////////////////////////////////////
// Helper functions.

#pragma region Helper functions

/// Compute a spline function. Depending on the value of `m_eSpline` this
/// will be either identity function, a cubic spline, or a quintic spline.
/// \param x A float in the range \f$[-1, 1]\f$.
/// \return The spline of \f$\mathsf{x}\f$ in the range \f$[-1, 1]\f$.

inline const float CPerlinNoise2D::spline(float x) const{
  assert(-1.0f <= x && x <= 1.0f);

  float fResult = 0.0f;

  switch(m_eSpline){
    case eSpline::None:    fResult = x;          break;
    case eSpline::Cubic:   fResult = spline3(x); break;
    case eSpline::Quintic: fResult = spline5(x); break;
  } //switch
  
  assert(-1.0f <= fResult && fResult <= 1.0f);

  return fResult;
} //spline

/// Perlin's pairing function, which combines two unsigned integers into one.
/// \param x First unsigned integer.
/// \param y Second unsigned integer.
/// \return An unsigned integer that depends upon x and y.

inline const size_t CPerlinNoise2D::pair(size_t x, size_t y) const{
  return hash(x) + y;
} //pair

/// Alternate version of Perlin's pairing function. Note: This function depends
/// on the implementation of `std::hash`. The C++ Standard does not require
/// that it be a good hash function, but we'll assume that it is. Most
/// implementations are.
/// \param x First number.
/// \param y Second number.
/// \return An unsigned integer that combines the parameters into a single number.

inline const size_t CPerlinNoise2D::pairstd(size_t x, size_t y) const{
  return (std::hash<size_t>{}(x) << 1) ^ std::hash<size_t>{}(y);
} //pairstd

/// Perlin's hash function, which uses a random permutation. Note that this
/// means that it repeats with a period of `m_nSize`.
/// \param x A number.
/// \return Hashed number in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hash(size_t x) const{
  return m_nPerm[x & m_nMask];
} //hash

/// A hash function using `std::hash`. Note: The C++ Standard does not require
/// that `std::hash` be a good hash function, but we'll assume that it is. Most
/// implementations are.
/// \param x A number.
/// \return Hashed number in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hashstd(size_t x) const{
  return std::hash<size_t>{}(x) & m_nMask;
} //hashstd

/// A stupid 2D hash function munged together by xor-ing two linear hash
/// functions together. For fixed primes \f$p_0, p_1, p_2\f$ and parameters
/// \f$x, y\f$, this function returns \f$(p_0x + p_1y) \bmod p_2 \f$ 
/// right-shifted by 8 bits and masked with `m_nMask`.
/// \param x A number.
/// \param y A number.
/// \return Hashed number in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hash2(size_t x, size_t y) const{
  const uint64_t p0 = 43214161; //prime
  const uint64_t p1 = 43216891; //prime
  const uint64_t p2 = 73202201; //prime
  
  const uint64_t h = (p0*(uint64_t)x + p1*(uint64_t)y)%p2; //hash

  return size_t(h >> 8) & m_nMask; //shift and mask
} //hash2

/// Get hash values at grid corners (at whole number coordinates).
/// \param x X-coordinate.
/// \param y Y-coordinate.
/// \param c [OUT] Array of four hash values for corners in row-major order.

void CPerlinNoise2D::HashCorners(size_t x, size_t y, size_t c[4]) const{
  switch(m_eHash){
    case eHash::Permutation:
      c[0] = hash(pair(x, y));     c[1] = hash(pair(x + 1, y));
      c[2] = hash(pair(x, y + 1)); c[3] = hash(pair(x + 1, y + 1));
    break;

    case eHash::LinearCongruential:
      c[0] = hash2(x, y);     c[1] = hash2(x + 1, y);
      c[2] = hash2(x, y + 1); c[3] = hash2(x + 1, y + 1);
    break;

    case eHash::Std:
      c[0] = hashstd(pairstd(x, y));     c[1] = hashstd(pairstd(x + 1, y));
      c[2] = hashstd(pairstd(x, y + 1)); c[3] = hashstd(pairstd(x + 1, y + 1));
    break;
  } //switch
} //HashCorners

/// For Perlin noise, multiply hashed gradient from `m_fTable` by coordinates. 
/// Use the hash value parameter to index into the gradient table for the
/// X gradient and rehash it for the index of the Y gradient. Add these
/// gradients multiplied by the fractional values of the position (that is,
/// return \f$z = x \frac{dz}{dx} + y\frac{dz}{dy}\f$). For Value noise, just
/// read the \f$z\f$ value directly from the table.
/// \param h Hash value for gradient table index.
/// \param x X-coordinate of point in the range \f$[-1, 1]\f$.
/// \param y Y-coordinate of point in the range \f$[-1, 1]\f$.
/// \return Corresponding Z-value.

inline const float CPerlinNoise2D::z(size_t h, float x, float y, eNoise t) const{
  assert(-1.0f <= x && x <= 1.0f);
  assert(-1.0f <= y && y <= 1.0f);
  assert(h == (h & m_nMask)); 

  float result = 0; //return result

  switch(t){ //noise type
    case eNoise::Perlin: 
      result = x*m_fTable[h] + y*m_fTable[hash(h)]; //gradient times position
      assert(-2.0f <= result && result <= 2.0f);
    break;
      
    case eNoise::Value:
      result = m_fTable[h]; //get value directly from table
      assert(-1.0f <= result && result <= 1.0f);
    break;
  } //switch

  return result; 
} //z

/// Linear interpolation of gradients or heights (depending on whether we're
/// generating Perlin or Value noise) along the X-axis.
/// \param sX Smoothed fractional part of X-coordinate.
/// \param fX Fractional part of X-coordinate (ignored in Value noise).
/// \param fY Fractional part of Y-coordinate (ignored in Value noise).
/// \param c Array of two gradients at grid points along X-axis.
/// \param t Noise type.
/// \return Linearly interpolated gradient or value.

const float CPerlinNoise2D::Lerp(float sX, float fX, float fY, size_t* c,
  eNoise t) const
{
  assert(-1.0f <= sX && sX <= 1.0f);
  assert( 0.0f <= fX && fX <= 1.0f);
  assert(-1.0f <= fY && fY <= 1.0f);

  const float result = lerp(sX, z(c[0], fX, fY, t), z(c[1], fX - 1, fY, t));
   
  switch(t){ //noise type
    case eNoise::Perlin: 
      //the worst case in the lerp above is fX and fX - 1 have the same
      //magnitude, that is, fX == 0.5f, in which case each z gets 1.0f from fY
      //and 0.5f from fX.
      assert(-1.5f <= result && result <= 1.5f);
      //return result/1.5f; 
      return result; 
    break;
      
    case eNoise::Value:
      assert(-1.0f <= result && result <= 1.0f);
      return result; 
    break;

    default: return 0.0f;
  } //switch
} //Lerp

#pragma endregion Helper functions

////////////////////////////////////////////////////////////////////////////////
// Noise generation functions.

#pragma region Noise generation functions

/// Compute a single octave of Perlin or Value noise at a 2D point.
/// \param x X-coordinate of point.
/// \param y Y-coordinate of point.
/// \param t Noise type.
/// \return A smoothed noise value in [-1, 1] at the given point.

const float CPerlinNoise2D::noise(float x, float y, eNoise t) const{
  const size_t nX = (size_t)floorf(x); //integer part of x
  const size_t nY = (size_t)floorf(y); //integer part of y

  const float fX = x - floorf(x); //fractional part of x
  const float fY = y - floorf(y); //fractional part of y

  //smooth fractional parts of x and y using spline curves
 
  const float sX = spline(fX); //apply spline curve to fractional part of x
  const float sY = spline(fY); //apply spline curve to fractional part of y
  
  //hash value at corners of enclosing grid square with integer coordinates

  size_t c[4] = {0}; //for hashed values at corners
  HashCorners(nX, nY, c); //get hashed values at corners

  //lerp along the top and bottom along the X-axis

  const float a = Lerp(sX, fX, fY, c, t);
  const float b = Lerp(sX, fX, fY - 1, &(c[2]), t);

  //now lerp these values along the Y-axis

  const float result = lerp(sY, a, b);
  assert(-1.0f <= result && result <= 1.0f);
  return result;
} //noise
  
/// Add multiple octaves of Perlin or Value noise to compute an effect similar
/// to turbulence at a single point. Each successive octave has its amplitude
/// multiplied by a value called the _lacunarity_ and its frequency multiplied
/// by a value called the _persistence_. These are usually set to 0.5 and 2.0,
/// respectively.
/// \param x X-coordinate of a 2D point.
/// \param y Y-coordinate of a 2D point.
/// \param t Noise type.
/// \param n Number of octaves.
/// \param alpha Lacunarity. Defaults to 0.5f.
/// \param beta Persistence. Defaults to 2.0f.
/// \return Turbulence value in \f$[-1, 1]\f$ at point \f$(\mathsf{x}, \mathsf{y})\f$.

const float CPerlinNoise2D::generate(float x, float y, eNoise t, size_t n, 
  float alpha, float beta) const
{
  assert(0.0f <= alpha && alpha < 1.0f);
  assert(beta > 1.0f);

  float sum = 0.0f; //for result
  float amplitude = 1.0f; //octave amplitude

  for(size_t i=0; i<n; i++){ //for each octave
    sum += amplitude*noise(x, y, t); //scale noise by amplitude
    amplitude *= alpha; //reduce amplitude by lacunarity  
    x *= beta; y *= beta; //multiply frequency by persistence
  } //for

  assert(amplitude == powf(alpha, (float)n));

  float result = (1 - alpha)*sum/(1 - amplitude); //sum of geometric progression

  if(t == eNoise::Perlin)result *= m_fSqrt2; //scale

  assert(-1.0f <= result && result <= 1.0f); //safety
  return result;
} //generate

#pragma endregion Noise generation functions
