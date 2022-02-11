/// \file CMain.h
/// \brief Interface for the main class CMain.

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

#ifndef __CMAIN_H__
#define __CMAIN_H__

#include "Includes.h"
#include "Windows.h"
#include "WindowsHelpers.h"
#include "perlin.h"

/// \brief The main class.
///
/// The interface between I/O from Windows (input from the drop-down menus,
/// output to the client area of the window), the noise generator, and the GDI+
/// graphics interface.

class CMain{
  private:
    HWND m_hWnd = nullptr; ///< Window handle.
    HMENU m_hFileMenu = nullptr; ///< Handle to the `File` menu.
    HMENU m_hGenMenu = nullptr; ///< Handle to the `Generate` menu.
    HMENU m_hDistMenu = nullptr; ///< Handle to the `Distribution` menu.
    HMENU m_hSplineMenu = nullptr; ///< Handle to the `Spline` menu.
    HMENU m_hSetMenu = nullptr; ///< Handle to the `Settings` menu.
    
    eNoise m_eNoise = eNoise::None; ///< Noise type.
    eDistribution m_eCurDist = eDistribution::Uniform; ///< Current distribution.
    eSpline m_eCurSpline = eSpline::Cubic; ///< Current spline function.
    UINT m_nSeed = 0; ///< Pseudorandom number seed.
    size_t m_nNumOctaves = 4; ///< Number of octaves of noise.
    float m_fScale = 64.0f; ///< Scale.
    UINT m_nLog2TableSize = 8; ///< Log base 2 of table size.

    ULONG_PTR m_gdiplusToken = 0; ///< GDI+ token.

    Gdiplus::Bitmap* m_pBitmap = nullptr; ///< Pointer to a bitmap image.
    CPerlinNoise2D* m_pPerlin = nullptr; ///< Pointer to Perlin noise generator.

    void CreateMenus(); ///< Create menus.
    void SetPixel(UINT, UINT, float); ///< Set pixel grayscale.
    void SetPixel(UINT, UINT, BYTE); ///< Set pixel grayscale.

  public:
    CMain(const HWND hwnd); ///< Constructor.
    ~CMain(); ///< Destructor.
    
    void CreateBitmap(int w, int h); ///< Create bitmap.

    void GeneratePixelNoise(); ///< Generate pixel noise.
    void GeneratePerlinNoise(eNoise); ///< Generate noise.
    void Regenerate(); ///< Generate with saved parameters.
    void Clear(); ///< Clear noise array.
    void Randomize(); ///< Randomize noise.

    void Initialize(eDistribution); ///< Set probability distribution.
    void SetSpline(eSpline); ///< Set spline function.

    void IncreaseOctaves(); ///< Increase number of octaves.
    void DecreaseOctaves(); ///< Decrease number of octaves.
    void IncreaseScale(); ///< Increase scale.
    void DecreaseScale(); ///< Decrease scale.
    void IncreaseTableSize(); ///< Increase table size.
    void DecreaseTableSize(); ///< Decrease table size.

    void OnPaint(); ///< Paint the client area of the window.
    Gdiplus::Bitmap* GetBitmap(); ///< Get pointer to bitmap.
    const std::wstring GetFileName() const; ///< Get save file name.
}; //CMain

#endif //__CMAIN_H__
