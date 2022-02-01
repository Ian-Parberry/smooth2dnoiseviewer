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

#pragma once

#include "Includes.h"

///////////////////////////////////////////////////////////////////////////////
// Menu IDs

#pragma region Menu IDs

#define IDM_FILE_SAVE 1 ///< Menu id for Save.
#define IDM_FILE_QUIT 2 ///< Menu id for Quit.

#define IDM_GENERATE_PIXELNOISE 3 ///< Menu id for Pixel Noise.
#define IDM_GENERATE_PERLINNOISE 4 ///< Menu id for Perlin Noise.
#define IDM_GENERATE_VALUENOISE 5 ///< Menu id for value Noise.

#pragma endregion Menu IDs

///////////////////////////////////////////////////////////////////////////////
// Helper functions

#pragma region Helper functions

//initialization functions

ULONG_PTR InitGDIPlus(); ///< Initialize GDI+.

//others

HRESULT SaveBitmap(HWND, Gdiplus::Bitmap*); ///< Save bitmap to file.

#pragma endregion Helper functions
