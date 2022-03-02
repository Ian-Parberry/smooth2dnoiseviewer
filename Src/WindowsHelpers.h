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
#include "Defines.h"

///////////////////////////////////////////////////////////////////////////////
// Menu IDs

#pragma region Menu IDs

#define IDM_FILE_SAVE  1 ///< Menu id for Save.
#define IDM_FILE_PROPS 2 ///< Menu id for Properties.
#define IDM_FILE_QUIT  3 ///< Menu id for Quit.

#define IDM_GENERATE_PERLINNOISE 4 ///< Menu id for Perlin Noise.
#define IDM_GENERATE_VALUENOISE  5 ///< Menu id for Value Noise.
#define IDM_GENERATE_RANDOMIZE   6 ///< Menu id for regenerate Noise.
#define IDM_GENERATE_JUMP        7 ///< Menu id for jump.
#define IDM_GENERATE_RESETORIGIN 8 ///< Menu id for reset origin.

#define IDM_VIEW_COORDS  9 ///< Menu id for view coordinates.
#define IDM_VIEW_GRID   10 ///< Menu id for view grid.

#define IDM_DISTRIBUTION_UNIFORM     11 ///< Menu id for uniform distribution.
#define IDM_DISTRIBUTION_COSINE      12 ///< Menu id for cosine distribution.
#define IDM_DISTRIBUTION_NORMAL      13 ///< Menu id for normal distribution.
#define IDM_DISTRIBUTION_EXPONENTIAL 14 ///< Menu id for exponential distribution.
#define IDM_DISTRIBUTION_MIDPOINT    15 ///< Menu id for midpoint displacement.
#define IDM_DISTRIBUTION_MAXIMAL     16 ///< Menu id for midpoint displacement.

#define IDM_HASH_PERM   17 ///< Menu id for permutation hash.
#define IDM_HASH_LCON   18 ///< Menu id for linear congruential hash.
#define IDM_HASH_STD    19 ///< Menu id for std::hash.

#define IDM_SPLINE_NONE    20 ///< Menu id for cubic spline.
#define IDM_SPLINE_CUBIC   21 ///< Menu id for no spline.
#define IDM_SPLINE_QUINTIC 22 ///< Menu id for quintic spline.

#define IDM_SETTINGS_OCTAVE_UP 23 ///< Menu id for octave up.
#define IDM_SETTINGS_OCTAVE_DN 24 ///< Menu id for octave down.
#define IDM_SETTINGS_SCALE_UP  25 ///< Menu id for scale up.
#define IDM_SETTINGS_SCALE_DN  26 ///< Menu id for scale down.
#define IDM_SETTINGS_TSIZE_UP  27 ///< Menu id for table size up.
#define IDM_SETTINGS_TSIZE_DN  28 ///< Menu id for table size down.
#define IDM_SETTINGS_RESET     29 ///< Menu id for reset settings.

#define IDM_HELP_HELP  30 ///< Menu id for display help.
#define IDM_HELP_ABOUT 31 ///< Menu id for display About info.

#pragma endregion Menu IDs

///////////////////////////////////////////////////////////////////////////////
// Helper functions

#pragma region Helper functions

//initialization functions

ULONG_PTR InitGDIPlus(); ///< Initialize GDI+.

//others

HRESULT SaveBitmap(HWND, const std::wstring&, Gdiplus::Bitmap*); ///< Save bitmap to file.

#pragma endregion Helper functions

///////////////////////////////////////////////////////////////////////////////
// Menu functions

#pragma region Menu functions

HMENU CreateFileMenu(HMENU); ///< Create `File` menu.
HMENU CreateGenerateMenu(HMENU); ///< Create `Generate` menu.
HMENU CreateViewMenu(HMENU); ///< Create `View` menu.
HMENU CreateDistributionMenu(HMENU); ///< Create `Distribution` menu.
HMENU CreateHashMenu(HMENU); ///< Create `Hash` menu.
HMENU CreateSplineMenu(HMENU); ///< Create `Spline` menu.
HMENU CreateSettingsMenu(HMENU); ///< Create `Settings` menu.
void CreateHelpMenu(HMENU); ///< Create `Help` menu.

void UpdateMenuItemGray(HMENU, UINT, eNoise, bool); ///< Update menu item bool.
void UpdateMenuItemCheck(HMENU, UINT, bool); ///< Update menu item check.

void UpdateFileMenu(HMENU, eNoise); ///< Update `File` menu.
void UpdateGenerateMenu(HMENU, eNoise); ///< Update `Generate` menu.
void UpdateViewMenu(HMENU, eNoise); ///< Update `View` menu.

void UpdateDistributionMenu(HMENU, eNoise, eDistribution); ///< Update `Distribution` menu.
void UpdateHashMenu(HMENU, eNoise, eHash); ///< Update `Hash` menu.
void UpdateSplineMenu(HMENU, eNoise, eSpline); ///< Update `Spline` menu.
void UpdateSettingsMenu(HMENU, eNoise); ///< Update `Settings` menu.

/// \brief Update a numeric menu item.
///
/// Update a menu item that has an up and a down, with maximum and minimum
/// values. Gray out the up item if the number is greater than the maximum
/// allowed, and gray out the down item if the number is less than the minimum
/// allowed. Gray out both if the noise type is `eNoise::None`.
/// \param hMenu Menu handle.
/// \param up Up menu item id.
/// \param dn Down menu item id.
/// \param noise Noise enumerated type.
/// \param n A number.
/// \param nMin Minimum value.
/// \param nMax Maximum value.

template<typename t> 
void UpdateMenuItem(HMENU hMenu, UINT up, UINT dn, eNoise noise, 
  t n, t nMin, t nMax)
{
  if(noise == eNoise::None){ //all gray
    EnableMenuItem(hMenu, up, MF_GRAYED);
    EnableMenuItem(hMenu, dn, MF_GRAYED);
  } //if

  else{ //grayed if out of range
    EnableMenuItem(hMenu, up, (n < nMax)? MF_ENABLED: MF_GRAYED);
    EnableMenuItem(hMenu, dn, (n > nMin)? MF_ENABLED: MF_GRAYED);
  } //else
} //UpdateMenuItem

#pragma endregion Menu functions

#endif //__WINDOWSHELPERS_H__
