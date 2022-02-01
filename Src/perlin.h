/// \file perlin.h
///
/// \brief Interface for the Perlin and Value noise generators.

#pragma once

void initPerlin(); ///< Initialize Perlin noise.
float PerlinNoise2D(float, float, float, float, unsigned); ///< Generate Perlin noise.
float ValueNoise2D(float, float, float, float, unsigned); ///< Generate Value noise.

