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

#include "Perlin.h"
#include "Helpers.h"
#include "Includes.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor and destructor.

#pragma region Constructor and destructor

/// Set the PRNG seed and initialize.

CPerlinNoise2D::CPerlinNoise2D(){  
  SetSeed();
  Initialize(); 
} //constructor

/// Deletes the permutation and gradient/value table.

CPerlinNoise2D::~CPerlinNoise2D(){
  delete [] m_fTable;
  delete [] m_nPerm;
} //destructor

/// Initialize the generator. Assumes that `m_nSize` has set initialized to
/// the table size, which must be a power of two. Create and initialize the
/// permutation and gradient/value tables. Initialize the bit mask.

void CPerlinNoise2D::Initialize(){
  assert(isPowerOf2(m_nSize)); //safety
  assert(m_nSize > 1); //safety

  m_nMask = m_nSize - 1;  //mask of n consecutive 1s
  m_nPerm = new size_t[m_nSize]; //permutation
  m_fTable = new float[m_nSize]; //gradients or height values

  RandomizeTable(m_eDistribution); //randomize gradient/value table
  RandomizePermutation(); //randomize permutation
} //Initialize

#pragma endregion Constructor and destructor

////////////////////////////////////////////////////////////////////////////////
// Functions that change noise settings.

#pragma region Functions that change noise settings

/// Use the standard algorithm to set the permutation in `m_nPerm` to a
/// pseudo-random permutation with each permutation equally likely .
/// The source of randomness is `std::default_random_engine`
/// with `std::uniform_int_distribution`. This function should be called
/// during initialization and when the table size changes. The pseudo-random
/// number generator is re-seeded from `m_nSeed` before the permutation is
/// generated. This means that the permutation used for each table size remains
/// the same until `m_nSeed` is changed.

void CPerlinNoise2D::RandomizePermutation(){
  for(size_t i=0; i<m_nSize; i++) //identity permutation
    m_nPerm[i] = i; 

  m_stdRandom.seed(m_nSeed); //reset PRNG

  for(size_t i=0; i<m_nSize; i++){ //randomize
    std::uniform_int_distribution<size_t> d(i, m_nSize - 1);
    std::swap(m_nPerm[i], m_nPerm[d(m_stdRandom)]);
  } //for
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
  assert(i < j && j < m_nSize);
  assert(alpha < 0.0f);

  std::uniform_real_distribution<float> d(-1.0f, 1.0f);

  if(j > i + 1){ //there is a midpoint to fill in
    const size_t mid = (i + j)/2; //mid point

    assert(i < mid && mid < j);

    const float fMean = (m_fTable[i] + m_fTable[j])/2.0f; //average of ends
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
  m_fTable[0] = 1.0f;
  m_fTable[m_nSize - 1] = -1.0f;

  RandomizeTableMidpoint(0, m_nSize - 1, 0.5f);
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
} //RandomizeTableMaximal

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
  std::exponential_distribution<float> d(4.0f);

  const size_t half = m_nSize/2;

  for(size_t i=0; i<half; i++) //positive values
    m_fTable[i] = clamp(0.0f, d(m_stdRandom), 1.0f);

  for(size_t i=half; i<m_nSize; i++) //negative values
    m_fTable[i] = -clamp(0.0f, d(m_stdRandom), 1.0f);
} //RandomizeTableExp

/// Set `m_fTable` to pseudo-random values in \f$[-1, 1]\f$ according 
/// to some probability distribution. The pseudo-random number generator is
/// re-seeded from `m_nSeed` before the table is generated. This means that the
/// table contents for each table size remains the same until `m_nSeed`
/// is changed.
/// \param d Probability distribution enumerated type.

void CPerlinNoise2D::RandomizeTable(eDistribution d){ 
  m_stdRandom.seed(m_nSeed); //reset PRNG
  m_eDistribution = d; //current distribution

  switch(d){
    case eDistribution::Uniform: RandomizeTableUniform();   break;
    case eDistribution::Maximal: RandomizeTableMaximal();   break;
    case eDistribution::Cosine:  RandomizeTableCos();       break;
    case eDistribution::Normal:  RandomizeTableNormal();    break;
    case eDistribution::Exponential: RandomizeTableExp();   break;
    case eDistribution::Midpoint: RandomizeTableMidpoint(); break;
  } //switch
} //RandomizeTable

/// Double the size of the permutation and gradient/value tables up to
/// a maximum of `m_nMaxTableSize` and call `Initialize()` to re-initialize.
/// \return true if the table size changed.

bool CPerlinNoise2D::DoubleTableSize(){
  if(m_nSize < m_nMaxTableSize){
    delete [] m_fTable;
    delete [] m_nPerm;
  
    m_nSize *= 2; //size must be a power of 2
    Initialize();
    return true;
  } //if

  return false;
} //DoubleTableSize

/// Halve the size of the permutation and gradient/value tables down to
/// a minimum of `m_nMinTableSize` and call `Initialize()` to re-initialize.
/// \return true if the table size changed.

bool CPerlinNoise2D::HalveTableSize(){
  if(m_nSize > m_nMinTableSize){
    delete [] m_fTable;
    delete [] m_nPerm;
  
    m_nSize /= 2; //size must be a power of 2
    Initialize();
    return true;
  } //if

  return false;
} //HalveTableSize

/// Set table size to the default and call `Initialize()` to re-initialize.
/// \return true if the table size changed.

bool CPerlinNoise2D::DefaultTableSize(){
  if(m_nSize != m_nDefTableSize){
    delete [] m_fTable;
    delete [] m_nPerm;
  
    m_nSize = m_nDefTableSize; 
    Initialize();
    return true;
  } //if

  return false;
} //DefaultTableSize

/// Set the pseudo-random number generator seed to `timeGetTime()`, the number
/// of milliseconds since Windows last rebooted. This should be sufficiently
/// unpredictable to make a good seed.

void CPerlinNoise2D::SetSeed(){ 
  m_nSeed = timeGetTime();
} //SetSeed

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

/// A 2D linear congruential hash function. For fixed primes \f$p_0, p_1, p_2\f$
/// and parameters \f$x, y\f$, this function returns
/// \f$(p_0x + p_1y) \bmod p_2 \f$ right-shifted by 8 bits and masked
/// with `m_nMask`.
/// \param x A number.
/// \param y A number.
/// \return Hashed number in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hash2(size_t x, size_t y) const{
  const uint64_t p0 = 11903454645187951493LL; //a prime
  const uint64_t p1 = 2078231835154824277LL; //another prime
  const uint64_t p2 = 5719147207009855033LL; //another prime
  
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
/// \return Smooth noise in \f$[-1, 1]\f$ at point \f$(\mathsf{x}, \mathsf{y})\f$.

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
  if(t == eNoise::Perlin)result *= 4.0f/3.0f; //scale up Perlin noise
  assert(-1.0f <= result && result <= 1.0f); //safety
  return result;
} //generate

#pragma endregion Noise generation functions

////////////////////////////////////////////////////////////////////////////////
// Reader functions.

#pragma region Reader functions

/// Reader function for the table size.
/// \return The table size.

const size_t CPerlinNoise2D::GetTableSize() const{
  return m_nSize;
} //GetTableSize

/// Reader function for the minimum table size.
/// \return The minimum table size.

const size_t CPerlinNoise2D::GetMinTableSize() const{
  return m_nMinTableSize;
} //GetMinTableSize

/// Reader function for the maximum table size.
/// \return The maximum table size.

const size_t CPerlinNoise2D::GetMaxTableSize() const{
  return m_nMaxTableSize;
} //GetMaxTableSize

/// Reader function for the default table size.
/// \return The default table size.

const size_t CPerlinNoise2D::GetDefTableSize() const{
  return m_nDefTableSize;
} //GetDefTableSize

/// Reader function for the hash function type.
/// \return The hash function type.

const eHash CPerlinNoise2D::GetHash() const{
  return m_eHash;
} //GetHash

/// Reader function for the spline function type.
/// \return The spline function type.

const eSpline CPerlinNoise2D::GetSpline() const{
  return m_eSpline;
} //GetSpline

/// Reader function for the distribution type.
/// \return The distribution type.

const eDistribution CPerlinNoise2D::GetDistribution() const{
  return m_eDistribution;
} //GetDistribution

#pragma endregion Reader functions
