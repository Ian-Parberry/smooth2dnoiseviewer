/// \file CMain.cpp
/// \brief Code for the main class CMain.

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

#include <random>
#include <algorithm>

#include "CMain.h"
#include "WindowsHelpers.h"
#include "Perlin.h"
#include "Defines.h"
#include "Helpers.h"

///////////////////////////////////////////////////////////////////////////////
// Constructors and destructors

#pragma region Constructors and destructors

/// Initialize GDI+, create the menus, seed the pseudorandom number generator
/// from `timeGetTime()`, and create the Perlin noise generator.
/// \param hwnd Window handle.

CMain::CMain(const HWND hwnd): m_hWnd(hwnd){
  m_gdiplusToken = InitGDIPlus(); //initialize GDI+
  CreateMenus(); //create the menu bar
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize); //Perlin noise generator
} //constructor

/// Delete the Perlin noise generator, delete the GDI+ objects, then shut
/// down GDI+.

CMain::~CMain(){
  delete m_pPerlin; //delete the Perlin noise generator
  delete m_pBitmap; //delete the bitmap
  Gdiplus::GdiplusShutdown(m_gdiplusToken); //shut down GDI+
} //destructor

#pragma endregion Constructors and destructors

///////////////////////////////////////////////////////////////////////////////
// Drawing functions

#pragma region Drawing functions

/// Draw the bitmap `m_pBitmap` to the window client area, scaled down if
/// necessary. This function should only be called in response to a WM_PAINT
/// message.

void CMain::OnPaint(){  
  PAINTSTRUCT ps; //paint structure
  HDC hdc = BeginPaint(m_hWnd, &ps); //device context
  Gdiplus::Graphics graphics(hdc); //GDI+ graphics object

  //bitmap width and height
  
  const int nBitmapWidth = m_pBitmap->GetWidth(); 
  const int nBitmapHeight = m_pBitmap->GetHeight(); 

  //get client rectangle

  RECT rectClient; //for client rectangle
  GetClientRect(m_hWnd, &rectClient); //get client rectangle
  const int nClientWidth  = rectClient.right - rectClient.left; //client width
  const int nClientHeight = rectClient.bottom - rectClient.top; //client height

  //compute destination rectangle

  const int nDestSide = min(nClientWidth, nClientHeight); //dest width and ht

  const int width  = min(nDestSide, nBitmapWidth); //dest rect width
  const int height = min(nDestSide, nBitmapHeight);  //dest rect height

  const int x = max(0, nClientWidth  - width)/2; //x margin
  const int y = max(0, nClientHeight - height)/2; //y margin

  Gdiplus::Rect rectDest(x, y, width, height); //destination rectangle

  //draw image to destination rectangle then clean up
  
  graphics.DrawImage(m_pBitmap, rectDest);

  EndPaint(m_hWnd, &ps); //this must be done last
} //OnPaint

#pragma endregion Drawing functions

///////////////////////////////////////////////////////////////////////////////
// Menu functions

#pragma region Menu functions

/// Add menus to the menu bar.

void CMain::CreateMenus(){
  HMENU hMenubar = CreateMenu();
  m_hFileMenu = CreateMenu();
  
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_SAVE,     L"Save...");
  EnableMenuItem(m_hFileMenu, IDM_FILE_SAVE, MF_GRAYED);
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_PROPS,     L"Properties...");
  EnableMenuItem(m_hFileMenu, IDM_FILE_PROPS, MF_GRAYED);
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_QUIT,     L"Quit");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hFileMenu, L"&File");
  
  m_hGenMenu = CreateMenu();

  AppendMenuW(m_hGenMenu, MF_STRING, IDM_GENERATE_PIXELNOISE,  L"Pixel noise");
  AppendMenuW(m_hGenMenu, MF_STRING, IDM_GENERATE_PERLINNOISE, L"Perlin noise");
  AppendMenuW(m_hGenMenu, MF_STRING, IDM_GENERATE_VALUENOISE,  L"Value noise");
  AppendMenuW(m_hGenMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(m_hGenMenu, MF_STRING, IDM_GENERATE_RANDOMIZE,   L"Randomize");
  EnableMenuItem(m_hGenMenu, IDM_GENERATE_RANDOMIZE, MF_GRAYED);

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hGenMenu, L"&Generate");
  
  m_hDistMenu = CreateMenu();

  AppendMenuW(m_hDistMenu, MF_STRING, IDM_DISTRIBUTION_UNIFORM, L"Uniform");
  CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_UNIFORM, MF_CHECKED);
  AppendMenuW(m_hDistMenu, MF_STRING, IDM_DISTRIBUTION_COSINE,  L"Cosine");
  AppendMenuW(m_hDistMenu, MF_STRING, IDM_DISTRIBUTION_NORMAL,  L"Normal");
  AppendMenuW(m_hDistMenu, MF_STRING, IDM_DISTRIBUTION_EXPONENTIAL, L"Exponential");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hDistMenu, L"&Distribution");

  m_hSplineMenu = CreateMenu();
  
  AppendMenuW(m_hSplineMenu, MF_STRING, IDM_SPLINE_NONE, L"None");
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_GRAYED);
  AppendMenuW(m_hSplineMenu, MF_STRING, IDM_SPLINE_CUBIC, L"Cubic");
  CheckMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_CHECKED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_GRAYED);
  AppendMenuW(m_hSplineMenu, MF_STRING, IDM_SPLINE_QUINTIC,  L"Quintic");
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_GRAYED);

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hSplineMenu, L"&Spline");

  m_hSetMenu = CreateMenu();
  
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_OCTAVE_UP, 
    L"Increase number of octaves");
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_OCTAVE_DN,
    L"Decrease number of octaves");
  AppendMenuW(m_hSetMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_SCALE_UP, L"Scale in");
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_SCALE_DN, L"Scale out");
  AppendMenuW(m_hSetMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_TSIZE_UP, 
    L"Increase table size");
  AppendMenuW(m_hSetMenu, MF_STRING, IDM_SETTINGS_TSIZE_DN, 
    L"Decrease table size");
  GrayOutSettingsMenu();
  
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hSetMenu, L"&Settings");

  SetMenu(m_hWnd, hMenubar);
} //CreateMenus

void CMain::GrayOutSettingsMenu(){
  if(m_eNoise == eNoise::None){
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_OCTAVE_UP, MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_OCTAVE_DN, MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_SCALE_UP, MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_SCALE_DN, MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_TSIZE_UP, MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_TSIZE_DN, MF_GRAYED);
  } //if

  else{
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_OCTAVE_UP, 
      (m_nNumOctaves < m_nMaxOctaves)? MF_ENABLED: MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_OCTAVE_DN,
      (m_nNumOctaves > m_nMinOctaves)? MF_ENABLED: MF_GRAYED);

    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_SCALE_UP,
      (m_fScale < m_fMaxScale)? MF_ENABLED: MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_SCALE_DN,
      (m_fScale > m_fMinScale)? MF_ENABLED: MF_GRAYED);

    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_TSIZE_UP,
      (m_nLog2TableSize < m_nMaxLog2TableSize)? MF_ENABLED: MF_GRAYED);
    EnableMenuItem(m_hSetMenu, IDM_SETTINGS_TSIZE_DN,
      (m_nLog2TableSize > m_nMinLog2TableSize)? MF_ENABLED: MF_GRAYED);
  } //else
} //GrayOutSettingsMenu

#pragma endregion Menu functions

///////////////////////////////////////////////////////////////////////////////
// Other functions

/// Create bitmap `m_pBitmap` and set all pixels to white.
/// \param w Bitmap width in pixels.
/// \param h Bitmap height in pixels.

void CMain::CreateBitmap(int w, int h){
  delete m_pBitmap;
  m_pBitmap = new Gdiplus::Bitmap(w, h);

  Gdiplus::Graphics graphics(m_pBitmap);
  graphics.Clear(Gdiplus::Color::White);
} //CreateBitmap

/// Set grayscale pixel in `m_pBitmap` from a floating point value in 
/// \f$[-1, 1]\f$, where \f$-1\f$ is black and \f$+1\f$ is white.
/// \param i Row number.
/// \param j Column number.
/// \param g Grayscale value in the range \f$[-1, 1]\f$.

void CMain::SetPixel(UINT i, UINT j, float g){
  SetPixel(i, j, BYTE(float(0xFF)*(g/2 + 0.5f)));
} //SetPixel

/// Set grayscale pixel in `m_pBitmap`.
/// \param i Row number.
/// \param j Column number.
/// \param b Grayscale value in the range \f$[0, 255]\f$.

void CMain::SetPixel(UINT i, UINT j, BYTE b){
  m_pBitmap->SetPixel(i, j, Gdiplus::Color(255, b, b, b));
} //SetPixel

/// Display dialog box containing noise properties.

void CMain::DisplayProperties(){
  std::wstring wstr;

  switch(m_eNoise){
    case eNoise::Pixel:  wstr = L"Pixel";  break;
    case eNoise::Perlin: wstr = L"Perlin"; break;
    case eNoise::Value:  wstr = L"Value";  break;
  } //switch
  
  wstr += L" noise, ";

  if(m_eNoise == eNoise::Perlin || m_eNoise == eNoise::Value)
    wstr += std::to_wstring(m_nNumOctaves) + L" octaves, ";

  switch(m_eCurDist){
    case eDistribution::Uniform: wstr += L"uniform"; break; //nothing   
    case eDistribution::Cosine:  wstr += L"cosine"; break;   
    case eDistribution::Normal:  wstr += L"normal"; break;    
    case eDistribution::Exponential: wstr += L"exponential"; break;
  } //switch

  wstr += L" distribution";

  if(m_eNoise != eNoise::Pixel){
    wstr += L", ";

    switch(m_eCurSpline){
      case eSpline::None: wstr += L"no"; break; 
      case eSpline::Cubic: wstr += L"cubic"; break; //nothing
      case eSpline::Quintic: wstr += L"quintic"; break;
    } //switch

    wstr += L" spline, scale ";
    wstr += std::to_wstring((size_t)round(m_fScale));

    wstr += L", table size ";
    wstr += std::to_wstring((size_t)roundf(pow(2, m_nLog2TableSize)));
  } //if

  wstr += L".";

  MessageBox(nullptr, wstr.c_str(), L"Properties", MB_ICONINFORMATION | MB_OK);
} //DisplayProperties

/// Generate pixel noise into the bitmap pointed to by `m_pBitmap`. The
/// lower byte returned by the C standard function `rand()` is used as
/// a grayscale value for each pixel. This function generates a new pattern
/// each time it is called.

void CMain::GeneratePixelNoise(){ 
  m_eNoise = eNoise::Pixel;

  EnableMenuItem(m_hFileMenu, IDM_FILE_PROPS, MF_ENABLED); 
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_PIXELNOISE, MF_CHECKED);
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_PERLINNOISE, MF_UNCHECKED);
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_VALUENOISE, MF_UNCHECKED);
  EnableMenuItem(m_hGenMenu, IDM_GENERATE_RANDOMIZE, MF_ENABLED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_GRAYED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_GRAYED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_GRAYED);
  GrayOutSettingsMenu();

  std::default_random_engine g;
  g.seed(m_nSeed);

  switch(m_eCurDist){
    case eDistribution::Uniform: {
      std::uniform_real_distribution<float> d(-1.0f, 1.0f);

      for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
        for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
          SetPixel(i, j, d(g));
    } //case
    break;
      
    case eDistribution::Cosine: {
      std::uniform_real_distribution<float> d(0.0f, PI);

      for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
        for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
          SetPixel(i, j, cosf(d(g)));
    } //case
    break;
      
    case eDistribution::Normal:{   
      std::normal_distribution<float> d(500.0f, 200.0f);

      for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
        for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
          SetPixel(i, j, 2.0f*clamp(0.0f, d(g)/1000.0f, 1.0f) - 1.0f);
    } //case
    break;
      
    case eDistribution::Exponential: {
      std::exponential_distribution<float> d(3.5f);

      for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
        for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
          SetPixel(i, j, ((rand() & 1)? -1.0f: 1.0f)*clamp(0.0f, d(g), 1.0f));
    } //case
    break;
  } //switch
} //GeneratePixelNoise

/// Generate Perlin noise into the bitmap pointed to by `m_pBitmap`.
/// \param t Type of Perlin noise.

void CMain::GeneratePerlinNoise(eNoise t){ 
  m_eNoise = t;

  EnableMenuItem(m_hFileMenu, IDM_FILE_PROPS, MF_ENABLED);
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_PIXELNOISE, MF_UNCHECKED);
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_PERLINNOISE, 
    (t == eNoise::Perlin)? MF_CHECKED: MF_UNCHECKED);
  CheckMenuItem(m_hGenMenu, IDM_GENERATE_VALUENOISE, 
    (t == eNoise::Value)? MF_CHECKED: MF_UNCHECKED);
  EnableMenuItem(m_hGenMenu, IDM_GENERATE_RANDOMIZE, MF_ENABLED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_ENABLED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_ENABLED);
  EnableMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_ENABLED);
  GrayOutSettingsMenu();

  for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
    for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
      SetPixel(i, j, m_pPerlin->generate(i/m_fScale, j/m_fScale,
        t, 0.5f, 2.0f, m_nNumOctaves));
} //GeneratePerlinNoise

/// Reader function for the bitmap pointer `m_pBitmap`.
/// \return The bitmap pointer `m_pBitmap`.

Gdiplus::Bitmap* CMain::GetBitmap(){
  return m_pBitmap;
} //GetBitmap

/// Clear the noise to white.

void CMain::Clear(){ 
  for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
    for(UINT j=0; j<m_pBitmap->GetHeight(); j++)
      SetPixel(i, j, 0.0f);
} //Clear

/// Randomize the noise generator and regenerate noise.

void CMain::Randomize(){
  m_nSeed = timeGetTime(); //for pixel noise
  m_pPerlin->Randomize(); //for Perlin and Value noise
  Regenerate();
} //Randomize

/// Generate last type of noise again.

void CMain::Regenerate(){
 switch(m_eNoise){
   case eNoise::Pixel:
     GeneratePixelNoise();
     break;

   case eNoise::Perlin:
   case eNoise::Value:
     GeneratePerlinNoise(m_eNoise);
     break;

   default:
     Clear();
     break;
 } //switch
} //Regenerate

/// Set Perlin noise probability distribution and regenerate noise.
/// \param d Probability distribution enumerated type.

void CMain::Initialize(eDistribution d){
  m_eCurDist = d;

  CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_UNIFORM, MF_UNCHECKED);
  CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_COSINE,  MF_UNCHECKED);
  CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_NORMAL,  MF_UNCHECKED);
  CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_EXPONENTIAL, MF_UNCHECKED);

  switch(d){
    case eDistribution::Uniform:
      CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_UNIFORM, MF_CHECKED);
      break;
      
    case eDistribution::Cosine:
      CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_COSINE, MF_CHECKED);
      break;
      
    case eDistribution::Normal:
      CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_NORMAL, MF_CHECKED);
      break;
      
    case eDistribution::Exponential:
      CheckMenuItem(m_hDistMenu, IDM_DISTRIBUTION_EXPONENTIAL, MF_CHECKED);
      break;
  } //switch

  m_pPerlin->Initialize(d);
  Regenerate();
} //Initialize

/// Set Perlin noise spline function and regenerate noise.
/// \param d Spline function enumerated type.

void CMain::SetSpline(eSpline d){
  m_eCurSpline = d;

  switch(d){
    case eSpline::None:
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_CHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_UNCHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_UNCHECKED);
      break;

    case eSpline::Cubic:
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_UNCHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_CHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_UNCHECKED);
      break;
      
    case eSpline::Quintic:
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_NONE, MF_UNCHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_CUBIC, MF_UNCHECKED);
      CheckMenuItem(m_hSplineMenu, IDM_SPLINE_QUINTIC, MF_CHECKED);
      break;  
  } //switch

  m_pPerlin->SetSpline(d);
  Regenerate();
} //SetSpline

/// Increase the number of octaves by one.

void CMain::IncreaseOctaves(){
  m_nNumOctaves = std::min<size_t>(m_nNumOctaves + 1, m_nMaxOctaves);
  Regenerate();
} //IncreaseOctaves

/// Decrease the number of octaves by one.

void CMain::DecreaseOctaves(){
  m_nNumOctaves = std::max<size_t>(m_nMinOctaves, m_nNumOctaves - 1);
  Regenerate();
} //DecreaseOctaves

/// Increase the scale.

void CMain::IncreaseScale(){
  m_fScale = std::min<float>(2.0f*m_fScale, m_fMaxScale);
  Regenerate();
} //IncreaseScale

/// Decrease the scale.

void CMain::DecreaseScale(){
  m_fScale = std::max<float>(m_fMinScale, m_fScale/2.0f);
  Regenerate();
} //DecreaseScale

/// Increase the table size.

void CMain::IncreaseTableSize(){
  m_nLog2TableSize = std::min<UINT>(m_nLog2TableSize + 1, m_nMaxLog2TableSize);
  delete m_pPerlin;
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize);
  Regenerate();
} //IncreaseTableSize

/// Decrease the table size, to a minimum of 8.

void CMain::DecreaseTableSize(){
  m_nLog2TableSize = std::max<UINT>(m_nMinLog2TableSize, m_nLog2TableSize - 1);
  delete m_pPerlin;
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize);
  Regenerate();
} //DecreaseTableSize

/// Get save file name from noise settings.
/// \param return Save file name without extension.

const std::wstring CMain::GetFileName() const{
  std::wstring wstr;

  switch(m_eNoise){
    case eNoise::Pixel:  wstr = L"Pixel";  break;
    case eNoise::Perlin: wstr = L"Perlin"; break;
    case eNoise::Value:  wstr = L"Value";  break;
  } //switch

  switch(m_eCurDist){
    case eDistribution::Uniform: break; //nothing   
    case eDistribution::Cosine:  wstr += L"-Cos"; break;   
    case eDistribution::Normal:  wstr += L"-Norm"; break;    
    case eDistribution::Exponential: wstr += L"-Exp"; break;
  } //switch

  if(m_eNoise != eNoise::Pixel)
    switch(m_eCurSpline){
      case eSpline::None: L"-NoSpline"; break; 
      case eSpline::Cubic: break; //nothing
      case eSpline::Quintic:wstr += L"-Quintic"; break;
    } //switch

  return wstr;
} //GetFileName

