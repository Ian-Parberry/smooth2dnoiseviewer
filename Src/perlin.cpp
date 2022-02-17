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
/// for a gradient table in Perlin noise and a height table in Value noise.
/// The permutation is initialized to a pseudo-random permutation from a
/// uniform distribution, The table is filled with pseudo-random values chosen
/// uniformly from \f$[-1,1]\f$.
/// \param n Number of consecutive 1 bits in the mask, log base 2 of the size.

CPerlinNoise2D::CPerlinNoise2D(size_t n):
  m_nSize((size_t)round(pow(2.0f, (float)n))), //size must be a power of 2
  m_nMask(m_nSize - 1), //mask of n consecutive 1s
  m_nPerm(new size_t[m_nSize]), //permutation
  m_fTable(new float[m_nSize]) //gradients or heights
{ 
  m_pRandom = new std::default_random_engine; 
  Initialize(eDistribution::Uniform);
  Randomize();
} //constructor

/// The destructor deletes the permutation and table created in
/// the constructor.

CPerlinNoise2D::~CPerlinNoise2D(){
  delete [] m_fTable;
  delete [] m_nPerm;
  delete m_pRandom;
} //destructor

#pragma endregion Constructor and destructor

////////////////////////////////////////////////////////////////////////////////
// Functions that change noise settings.

#pragma region Functions that change noise settings

/// Set the permutation in `m_nPerm` to a pseudo-random permutation with
/// each permutation equally likely. The standard algorithm is used, with the
/// C standard library function `rand()` used as a source of randomness.

void CPerlinNoise2D::Randomize(){
  m_nSeed = timeGetTime(); ///< Pseudorandom number seed.

  for(size_t i=0; i<m_nSize; i++) //identity permutation
    m_nPerm[i] = i; 

  for(size_t i=0; i<m_nSize; i++) //randomize
    std::swap(m_nPerm[i], m_nPerm[i + rand()%(m_nSize - i)]);
} //Randomize

/// Initialize a chunk of the gradient/height table `m_fTable` using midpoint
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
/// the table chunk.
/// \param i Lower index.
/// \param j Upper index.
/// \param alpha Lacunarity.

void CPerlinNoise2D::MidpointDisplacement(size_t i, size_t j, float alpha){
  std::uniform_real_distribution<float> d(-1.0f, 1.0f);

  if(j > i + 1){ //base of recursion is "do nothing"
    const size_t mid = (i + j)/2; //mid point

    const float fMean = (m_fTable[i] + m_fTable[j - 1])/2.0f; //average of ends
    const float fRand = d(*m_pRandom); //(float)rand()/RAND_MAX; //random value in [0, 1]
    //const float fOffset = alpha*(2.0f*fRand - 1.0f); //random offset
    m_fTable[mid] = clamp(-1.0f, fMean + fRand, 1.0f); //mid point is average plus offset
    alpha *= 0.5f; //increase lacunarity

    MidpointDisplacement(i, mid, alpha); //recurse on first half
    MidpointDisplacement(mid, j, alpha); //recurse on second half
  } //if
} //MidpointDisplacement

/// Initialize `m_fTable` to pseudo-random values in \f$[-1, 1]\f$ according 
/// to some probability distribution. The pseudo-random number generators are
/// seeded from `m_nSeed`, which means that the table contents are the same each
/// time this function is called with the same parameter, up until `Randomize()`
/// is called.
/// \param d Probability distribution enumerated type.

void CPerlinNoise2D::Initialize(eDistribution d){
  switch(d){
    case eDistribution::Uniform: {   
      std::uniform_real_distribution<float> d(-1.0f, 1.0f);

      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = d(*m_pRandom);
    } //case
    break;

    case eDistribution::Cosine: {
      std::uniform_real_distribution<float> d(0.0f, 1.0f);

      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = cosf(PI*d(*m_pRandom));
    } //case
    break;
    
    case eDistribution::Normal: {  
      std::normal_distribution<float> d(500.0f, 200.0f);

      for(size_t i=0; i<m_nSize; i++)
        m_fTable[i] = 2.0f*clamp(0.0f, d(*m_pRandom)/1000.0f, 1.0f) - 1.0f;
    } //case
    break;

    case eDistribution::Exponential: {
      std::exponential_distribution<float> d(8.0f);

      const size_t half = m_nSize/2;

      for(size_t i=0; i<half; i++) //positive values
        m_fTable[i] = clamp(0.0f, d(*m_pRandom), 1.0f);

      for(size_t i=half; i<m_nSize; i++) //negative values
        m_fTable[i] = -clamp(0.0f, d(*m_pRandom), 1.0f);
    } //case
    break;

    case eDistribution::MidpointDisplacement:
      m_fTable[0] = 1.0f;
      m_fTable[m_nSize - 1] = -1.0f;

      MidpointDisplacement(0, m_nSize, 0.5f);
    break;
  } //switch
} //Initialize

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
  float fResult = 0.0f;

  switch(m_eSpline){
    case eSpline::None:    fResult = x;          break;
    case eSpline::Cubic:   fResult = spline3(x); break;
    case eSpline::Quintic: fResult = spline5(x); break;
  } //switch

  return fResult;
} //spline

/// Apply hashed gradient from `m_fTable` to a point.
/// \param h Hash value for X and Y gradient.
/// \param x X-coordinate of point in the range \f$[0, 1]\f$.
/// \param y Y-coordinate of point in the range \f$[0, 1]\f$.
/// \return The X and Y gradients times the point in the range \f$[-1, 1]\f$.

inline const float CPerlinNoise2D::grad(size_t h, float x, float y) const{
  h = h & m_nMask; //safety
  return clamp(-1.0f, x*m_fTable[h] + y*m_fTable[hash(h)], 1.0f);
} //grad

/// Perlin's pairing function, which combines two unsigned integers into one.
/// \param x First unsigned integer.
/// \param y Second unsigned integer.
/// \return An unsigned integer that depends upon x and y.

inline const size_t CPerlinNoise2D::pair(size_t x, size_t y) const{
  return hash(x) + y;
} //pair

/// Alternate version of Perlin's pairing function using `hash2()` instead
/// of `hash1()`.
/// \param x First unsigned integer.
/// \param y Second unsigned integer.
/// \return An unsigned integer that depends upon x and y.

inline const size_t CPerlinNoise2D::pair2(size_t x, size_t y) const{
  return hash2(x) + y;
} //pair

/// Perlin's hash function, which uses a random permutation. Note that this
/// means that it repeats with a period of `m_nSize`.
/// \param x An unsigned integer.
/// \return A hash value in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hash(size_t x) const{
  return m_nPerm[x & m_nMask];
} //hash

/// A hash function using modular arithmetic and exclusive-or.
/// \param x An unsigned integer.
/// \return A hash value in the range [0, `m_nSize` - 1].

inline const size_t CPerlinNoise2D::hash2(size_t x) const{
  const size_t p0 = 9973; //a prime number
  const size_t p1 = 2147483647; //another prime number

  x = p0*x + p1; //modular arithmetic using the word-size as modulus
  return (x ^ (x*x)) & m_nMask; //exclusive-or and mask result
} //hash

/// Get hash values at grid corners (at whole number coordinates).
/// \param x X-coordinate.
/// \param y Y-coordinate.
/// \param c [OUT] Array of four hash values for corners in row-major order.

void CPerlinNoise2D::HashCorners(size_t x, size_t y, size_t c[4]) const{
  switch(m_eHash){
    case eHash::Permutation:
      c[0] = hash(pair(x,     y)); 
      c[1] = hash(pair(x + 1, y));
      c[2] = hash(pair(x,     y + 1)); 
      c[3] = hash(pair(x + 1, y + 1));
    break;

    case eHash::Arithmetic:
      c[0] = hash2(pair2(x,     y)); 
      c[1] = hash2(pair2(x + 1, y));
      c[2] = hash2(pair2(x,     y + 1)); 
      c[3] = hash2(pair2(x + 1, y + 1));
    break;
  } //switch
} //HashCorners

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

  //lerp along the top and bottom in the x direction

  float a=0, b=0; //for lerped values

  switch(t){ //depending on whether Perlin noise or Value noise
    case eNoise::Perlin: //lerp gradients
      a = lerp(sX, grad(c[0], fX, fY),     grad(c[1], fX - 1, fY));
      b = lerp(sX, grad(c[2], fX, fY - 1), grad(c[3], fX - 1, fY - 1));
    break;
      
    case eNoise::Value: //lerp heights
      a = lerp(sX, m_fTable[c[0]], m_fTable[c[1]]);
      b = lerp(sX, m_fTable[c[2]], m_fTable[c[3]]);
    break;
  } //switch

  //now lerp in the other direction

  return lerp(sY, a, b);
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
  float sum = 0.0f; //for result
  float amplitude = 1.0f;

  for(size_t i=0; i<n; i++){ //for each octave
    sum += amplitude*noise(x, y, t); //scale noise by amplitude
    amplitude *= alpha; //reduce amplitude by lacunarity  
    x *= beta; y *= beta; //multiply frequency by persistence
  } //for

  //scale result by sum of geometric progression
  const float result = (1 - alpha)*sum/(1 - amplitude); 

  //scale up for Perlin noise
  return (t == eNoise::Perlin)? SQRT2*result: result;
} //generate

#pragma endregion Noise generation functions
