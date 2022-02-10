/// \file WindowsHelpers.h
/// \brief Interface for some helpful Windows-specific functions.
///
/// These platform-dependent functions are hidden away so that the faint-of-heart
/// don't have to see them if they're offended by them. 

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

#ifndef __WINDOWSHELPERS_H__
#define __WINDOWSHELPERS_H__

#include "Includes.h"

///////////////////////////////////////////////////////////////////////////////
// Menu IDs

#pragma region Menu IDs

#define IDM_FILE_SAVE 1 ///< Menu id for Save.
#define IDM_FILE_QUIT 2 ///< Menu id for Quit.

#define IDM_GENERATE_PIXELNOISE  3 ///< Menu id for Pixel Noise.
#define IDM_GENERATE_PERLINNOISE 4 ///< Menu id for Perlin Noise.
#define IDM_GENERATE_VALUENOISE  5 ///< Menu id for Value Noise.
#define IDM_GENERATE_RANDOMIZE   6 ///< Menu id for randomize.

#define IDM_DISTRIBUTION_UNIFORM  7 ///< Menu id for uniform distribution.
#define IDM_DISTRIBUTION_COSINE   8 ///< Menu id for cosine distribution.
#define IDM_DISTRIBUTION_NORMAL   9 ///< Menu id for normal distribution.
#define IDM_DISTRIBUTION_EXPONENTIAL  10 ///< Menu id for exponential distribution.

#define IDM_SPLINE_CUBIC    11 ///< Menu id for cubic spline.
#define IDM_SPLINE_QUINTIC  12 ///< Menu id for quintic spline.
#pragma endregion Menu IDs

///////////////////////////////////////////////////////////////////////////////
// Helper functions

#pragma region Helper functions

//initialization functions

ULONG_PTR InitGDIPlus(); ///< Initialize GDI+.

//others

HRESULT SaveBitmap(HWND, const std::wstring&, Gdiplus::Bitmap*); ///< Save bitmap to file.

#pragma endregion Helper functions

#endif //__WINDOWSHELPERS_H__
