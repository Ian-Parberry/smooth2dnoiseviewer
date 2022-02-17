/// \file WindowsHelpers.cpp
/// \brief Code for some helpful Windows-specific functions.
///
/// These platform-dependent functions are hidden away so that the faint-of-heart
/// don't have to see them if they can't handle it. 

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

#include <shobjidl_core.h>
#include <atlbase.h>

#include "WindowsHelpers.h"
#include "Includes.h"

///////////////////////////////////////////////////////////////////////////////
// Initialization functions

#pragma region Initialization

/// Initialize GDI+ and get a GDI+ token.
/// \return A GDI+ token.

ULONG_PTR InitGDIPlus(){
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
  return gdiplusToken;
} //InitGDIPlus

#pragma endregion Initialization

///////////////////////////////////////////////////////////////////////////////
// Save functions

#pragma region Save functions

/// Get an encoder clsid for an image file format.
/// \param format File format using wide characters.
/// \param pClsid [OUT] Pointer to clsid.
/// \return S_OK for success, E_FAIL for failure.

HRESULT GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
  UINT num = 0; //number of image encoders
  UINT n = 0; //size of the image encoder array in bytes
  HRESULT hr = E_FAIL; //return result

  Gdiplus::ImageCodecInfo* pCodecInfo = nullptr; //for codec info
  if(FAILED(Gdiplus::GetImageEncodersSize(&num, &n)))return E_FAIL; //get sizes
  if(n == 0)return E_FAIL; //there are no encoders

  pCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(n)); //allocate codec info memory
  if(pCodecInfo == nullptr)return E_FAIL; //malloc failed (as if)
  if(FAILED(GetImageEncoders(num, n, pCodecInfo)))return E_FAIL; //get encoders

  for(UINT j=0; j<num && hr!=S_OK; j++) //for each encoder, while not found
    if(wcscmp(pCodecInfo[j].MimeType, format) == 0){ //found the codex we want
      *pClsid = pCodecInfo[j].Clsid; //return it
      hr = S_OK; //success
    } //if

  free(pCodecInfo); //clean up
  return hr;
} //GetEncoderClsid

/// Display a `Save` dialog box for png files and save a bitmap to the file name
/// that the user selects. Only files with a `.png` extension are allowed. The
/// default file name is "ImageN.png", where N is the number of images saved
/// so far in the current instance of this program. This prevents any collisions
/// with files already saved by this instance. If there is a collision with a
/// file from a previous instance, then the user is prompted to overwrite or
/// rename it in the normal fashion. 
/// \param hwnd Window handle.
/// \param wstrName File name without extension.
/// \param pBitmap Pointer to a bitmap.
/// \return S_OK for success, E_FAIL for failure.

HRESULT SaveBitmap(HWND hwnd, const std::wstring& wstrName, 
  Gdiplus::Bitmap* pBitmap)
{
  COMDLG_FILTERSPEC filetypes[] = { //png files only
    {L"PNG Files", L"*.png"}
  }; //filetypes

  std::wstring wstrFileName; //result
  CComPtr<IFileSaveDialog> pDlg; //pointer to save dialog box
  CComPtr<IShellItem> pItem; //item pointer
  LPWSTR pwsz = nullptr; //pointer to null-terminated wide string for result

  //fire up the save dialog box
 
  if(FAILED(pDlg.CoCreateInstance(__uuidof(FileSaveDialog))))return E_FAIL; 

  pDlg->SetFileTypes(_countof(filetypes), filetypes); //set file types to png
  pDlg->SetTitle(L"Save Image"); //set title bar text
  pDlg->SetFileName(wstrName.c_str()); //set default file name
  pDlg->SetDefaultExtension(L"png"); //set default extension
 
  if(FAILED(pDlg->Show(hwnd)))return E_FAIL; //show the dialog box     
  if(FAILED(pDlg->GetResult(&pItem)))return E_FAIL; //get the result item
  if(FAILED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz)))return E_FAIL; //get file name 

  //wstrFileName should now contain the selected file name

  CLSID clsid; //for PNG class id
  if(FAILED(GetEncoderClsid((WCHAR*)L"image/png", &clsid)))return E_FAIL; //get
  pBitmap->Save(pwsz, &clsid, nullptr); //the actual save happens here

  CoTaskMemFree(pwsz); //clean up

  return S_OK;
} //SaveBitmap

#pragma endregion Save functions

///////////////////////////////////////////////////////////////////////////////
// Create menu functions

#pragma region Create menu functions

/// Create `File` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `File` menu.

HMENU CreateFileMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();
  
  AppendMenuW(hMenu, MF_STRING, IDM_FILE_SAVE,  L"Save...");
  AppendMenuW(hMenu, MF_STRING, IDM_FILE_PROPS, L"Properties...");
  AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT,  L"Quit");
  
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&File");
  return hMenu;
} //CreateFileMenu

/// Create `Generate` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `Generate` menu.

HMENU CreateGenerateMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();

  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_PERLINNOISE, L"Perlin noise");
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_VALUENOISE,  L"Value noise");
  AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_JUMP, L"Jump");
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_RESETORIGIN, L"Reset origin");
  AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_RANDOMIZE,  L"Randomize");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Generate");
  return hMenu;
} //CreateGenerateMenu

/// Create `Distribution` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `Distribution` menu.

HMENU CreateDistributionMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();

  AppendMenuW(hMenu, MF_STRING, IDM_DISTRIBUTION_UNIFORM, L"Uniform");
  AppendMenuW(hMenu, MF_STRING, IDM_DISTRIBUTION_COSINE,  L"Cosine");
  AppendMenuW(hMenu, MF_STRING, IDM_DISTRIBUTION_NORMAL,  L"Normal");
  AppendMenuW(hMenu, MF_STRING, IDM_DISTRIBUTION_EXPONENTIAL, L"Exponential");
  AppendMenuW(hMenu, MF_STRING, IDM_DISTRIBUTION_MIDPOINT, L"Midpoint displacement");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Distribution");
  return hMenu;
} //CreateDistributionMenu

/// Create `Hash` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `Hash` menu.

HMENU CreateHashMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();

  AppendMenuW(hMenu, MF_STRING, IDM_HASH_PERM,  L"Permutation");
  AppendMenuW(hMenu, MF_STRING, IDM_HASH_STD,   L"Std::hash");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Hash");
  return hMenu;
} //CreateHashMenu

/// Create `Spline` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `Spline` menu.

HMENU CreateSplineMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();

  AppendMenuW(hMenu, MF_STRING, IDM_SPLINE_NONE,    L"None");
  AppendMenuW(hMenu, MF_STRING, IDM_SPLINE_CUBIC,   L"Cubic");
  AppendMenuW(hMenu, MF_STRING, IDM_SPLINE_QUINTIC, L"Quintic");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Spline");
  return hMenu;
} //CreateSplineMenu

/// Create `Settings` menu.
/// \param hMenubar Handle to menu bar.
/// \return Handle to `Settings` menu.

HMENU CreateSettingsMenu(HMENU hMenubar){
  HMENU hMenu = CreateMenu();
  
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_OCTAVE_UP, 
    L"Increase number of octaves");
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_OCTAVE_DN,
    L"Decrease number of octaves");
  AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_SCALE_UP, L"Scale up");
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_SCALE_DN, L"Scale down");
  AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_TSIZE_UP,  L"Table size up");
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_TSIZE_DN, L"Table size down");
  AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS_RESET, L"Reset to defaults");
  
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Settings");
  return hMenu;
} //CreateSettingsMenu

#pragma endregion Create menu functions

///////////////////////////////////////////////////////////////////////////////
// Update menu functions

#pragma region Update menu functions

/// Gray out the `Properties` and `Save` menu entries in the `File` menu if
/// there is no noise present, and ungray them otherwise.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.

void UpdateFileMenu(HMENU hMenu, eNoise noise){
  if(noise == eNoise::None){
    EnableMenuItem(hMenu, IDM_FILE_SAVE,  MF_GRAYED);
    EnableMenuItem(hMenu, IDM_FILE_PROPS, MF_GRAYED);
  } //if

  else{
    EnableMenuItem(hMenu, IDM_FILE_SAVE,  MF_ENABLED);
    EnableMenuItem(hMenu, IDM_FILE_PROPS, MF_ENABLED);
  } //else
} //UpdateFileMenu

/// Gray out and set the checkmarks in the `Generate` menu according to the
/// current noise properties. Check or uncheck the menu entries for pixel,
/// Perlin, and Value noise depending on the current noise type. Gray out the
/// `Randomize` menu entry if there is no noise generated, and ungray it
/// otherwise.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.

void UpdateGenerateMenu(HMENU hMenu, eNoise noise){
  CheckMenuItem(hMenu, IDM_GENERATE_PERLINNOISE, 
    (noise == eNoise::Perlin)? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_GENERATE_VALUENOISE,
    (noise == eNoise::Value)? MF_CHECKED: MF_UNCHECKED);
  
  EnableMenuItem(hMenu, IDM_GENERATE_RANDOMIZE, 
    (noise == eNoise::None)? MF_GRAYED: MF_ENABLED);
  EnableMenuItem(hMenu, IDM_GENERATE_JUMP, 
    (noise == eNoise::None)? MF_GRAYED: MF_ENABLED);

  EnableMenuItem(hMenu, IDM_GENERATE_RESETORIGIN, 
    (noise == eNoise::None)? MF_GRAYED: MF_ENABLED);
} //UpdateGenerateMenu

/// Gray or ungray a menu item depending on noise type and a boolean value.
/// \param hMenu Menu handle.
/// \param item Menu item id.
/// \param noise Noise enumerated type.
/// \param bGray True if entry is to be grayed out.

void UpdateMenuItemBool(HMENU hMenu, UINT item, eNoise noise, bool bGray){
  EnableMenuItem(hMenu, item, 
    (noise == eNoise::None || bGray)? MF_GRAYED: MF_ENABLED);
} //UpdateMenuItemBool
  
/// Gray out and set the checkmarks in the `Distribution` menu according to the
/// current noise and distribution types.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.
/// \param distr Distribution enumerated type.

void UpdateDistributionMenu(HMENU hMenu, eNoise noise, eDistribution distr){
  if(noise == eNoise::None){
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_UNIFORM, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_COSINE, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_NORMAL, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_EXPONENTIAL, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_MIDPOINT, MF_GRAYED);
  } //if

  else{
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_UNIFORM, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_COSINE, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_NORMAL, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_EXPONENTIAL, MF_ENABLED);
    EnableMenuItem(hMenu, IDM_DISTRIBUTION_MIDPOINT, MF_ENABLED);
  } //if

  CheckMenuItem(hMenu, IDM_DISTRIBUTION_UNIFORM, 
    distr == eDistribution::Uniform? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_DISTRIBUTION_COSINE, 
    distr == eDistribution::Cosine? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_DISTRIBUTION_NORMAL, 
    distr == eDistribution::Normal? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_DISTRIBUTION_EXPONENTIAL, 
    distr == eDistribution::Exponential? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_DISTRIBUTION_MIDPOINT, 
    distr == eDistribution::MidpointDisplacement? MF_CHECKED: MF_UNCHECKED);
} //UpdateDistributionMenu
  
/// Gray out and set the checkmarks in the `Hash` menu according to the
/// current noise and hash function types.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.
/// \param h Hash function enumerated type.

void UpdateHashMenu(HMENU hMenu, eNoise noise, eHash h){
  switch(noise){
    case eNoise::None: 
      EnableMenuItem(hMenu, IDM_HASH_PERM,  MF_GRAYED);
      EnableMenuItem(hMenu, IDM_HASH_STD,   MF_GRAYED);
    break;

    case eNoise::Perlin:
    case eNoise::Value:
      EnableMenuItem(hMenu, IDM_HASH_PERM,  MF_ENABLED);
      EnableMenuItem(hMenu, IDM_HASH_STD,   MF_ENABLED);
    break;
  } //switch

  CheckMenuItem(hMenu, IDM_HASH_PERM,
    (h == eHash::Permutation)? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_HASH_STD,
    (h == eHash::Std)? MF_CHECKED: MF_UNCHECKED);
} //UpdateHashMenu

/// Gray out and set the checkmarks in the `Spline` menu according to the
/// current noise and spline types.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.
/// \param spline Spline enumerated type.

void UpdateSplineMenu(HMENU hMenu, eNoise noise, eSpline spline){
  switch(noise){
    case eNoise::None: 
      EnableMenuItem(hMenu, IDM_SPLINE_NONE,     MF_GRAYED);
      EnableMenuItem(hMenu, IDM_SPLINE_CUBIC,    MF_GRAYED);
      EnableMenuItem(hMenu, IDM_SPLINE_QUINTIC,  MF_GRAYED);
    break;

    case eNoise::Perlin:
    case eNoise::Value:
      EnableMenuItem(hMenu, IDM_SPLINE_NONE,     MF_ENABLED);
      EnableMenuItem(hMenu, IDM_SPLINE_CUBIC,    MF_ENABLED);
      EnableMenuItem(hMenu, IDM_SPLINE_QUINTIC,  MF_ENABLED);
    break;
  } //switch

  CheckMenuItem(hMenu, IDM_SPLINE_NONE,
    (spline == eSpline::None)? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_SPLINE_CUBIC,
    (spline == eSpline::Cubic)? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_SPLINE_QUINTIC,
    (spline == eSpline::Quintic)? MF_CHECKED: MF_UNCHECKED);
} //UpdateSplineMenu

/// Gray out entries in the `Settings` menu if they are not appropriate for the
/// current noise type and parameters.
/// \param hMenu Menu handle.
/// \param noise Noise enumerated type.

void UpdateSettingsMenu(HMENU hMenu, eNoise noise){
  if(noise == eNoise::None){
    EnableMenuItem(hMenu, IDM_SETTINGS_OCTAVE_UP, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SETTINGS_OCTAVE_DN, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SETTINGS_SCALE_UP,  MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SETTINGS_SCALE_DN,  MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SETTINGS_TSIZE_UP,  MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SETTINGS_TSIZE_DN,  MF_GRAYED);
  } //if
} //UpdateSettingsMenu
