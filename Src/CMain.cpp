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

/// Initialize GDI+, create the menus, and create the Perlin noise generator.
/// \param hwnd Window handle.

CMain::CMain(const HWND hwnd): m_hWnd(hwnd){
  m_gdiplusToken = InitGDIPlus(); //initialize GDI+
  CreateMenus(); //create the menu bar
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize); //Perlin noise generator
} //constructor

/// Delete the Perlin noise generator, delete the GDI+ objects, shut down GDI+.

CMain::~CMain(){
  delete m_pPerlin; //delete the Perlin noise generator
  delete m_pBitmap; //delete the bitmap
  Gdiplus::GdiplusShutdown(m_gdiplusToken); //shut down GDI+
} //destructor

#pragma endregion Constructors and destructors

///////////////////////////////////////////////////////////////////////////////
// Drawing functions

#pragma region Drawing functions

/// Draw the bitmap pointed to by `m_pBitmap` to the window client area,
/// scaled down if necessary. This function should only be called in response
/// to a `WM_PAINT` message.

void CMain::OnPaint(){  
  PAINTSTRUCT ps; //paint structure
  HDC hdc = BeginPaint(m_hWnd, &ps); //device context
  Gdiplus::Graphics graphics(hdc); //GDI+ graphics object

  //get bitmap width and height
  
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

/// Create and add menus to the menu bar.

void CMain::CreateMenus(){
  HMENU hMenubar = CreateMenu();

  m_hFileMenu = CreateFileMenu(hMenubar);
  m_hGenMenu = CreateGenerateMenu(hMenubar);
  m_hDistMenu = CreateDistributionMenu(hMenubar);
  m_hHashMenu = CreateHashMenu(hMenubar);
  m_hSplineMenu = CreateSplineMenu(hMenubar);
  m_hSetMenu = CreateSettingsMenu(hMenubar);

  SetMenu(m_hWnd, hMenubar);
  UpdateMenus();
} //CreateMenus

/// Update all menus, that is, gray out inactive menu items and place a 
/// checkmark next to chosen menu items.

void CMain::UpdateMenus(){
  UpdateFileMenu(m_hFileMenu, m_eNoise); 

  UpdateGenerateMenu(m_hGenMenu, m_eNoise);
  UpdateMenuItemBool(m_hGenMenu, IDM_GENERATE_RESETORIGIN, m_eNoise,
    m_fOriginX == 0.0f && m_fOriginY == 0.0f);

  UpdateDistributionMenu(m_hDistMenu, m_eNoise, m_eDistr); 
  UpdateHashMenu(m_hHashMenu, m_eNoise, m_eHash); 
  UpdateSplineMenu(m_hSplineMenu, m_eNoise, m_eSpline); 

  UpdateSettingsMenu(m_hSetMenu, m_eNoise); 

  UpdateMenuItem(m_hSetMenu, 
    IDM_SETTINGS_OCTAVE_UP, IDM_SETTINGS_OCTAVE_DN, m_eNoise, 
    m_nOctaves, m_nMinOctaves, m_nMaxOctaves);
  UpdateMenuItem(m_hSetMenu,
     IDM_SETTINGS_OCTAVE_UP, IDM_SETTINGS_OCTAVE_DN, m_eNoise, 
    m_nOctaves, m_nMinOctaves, m_nMaxOctaves);  
  UpdateMenuItem(m_hSetMenu, 
    IDM_SETTINGS_SCALE_UP, IDM_SETTINGS_SCALE_DN, m_eNoise, 
    m_fScale, m_fMinScale, m_fMaxScale);  
  UpdateMenuItem(m_hSetMenu, 
    IDM_SETTINGS_TSIZE_UP, IDM_SETTINGS_TSIZE_DN, m_eNoise, 
    m_nLog2TableSize, m_nMinLog2TableSize, m_nMaxLog2TableSize); 
  UpdateMenuItemBool(m_hSetMenu, IDM_SETTINGS_RESET, m_eNoise,
    m_nOctaves == m_nDefOctaves && m_fScale == m_fDefScale
    && m_nLog2TableSize == m_nDefLog2TableSize); 
} //UpdateMenus

#pragma endregion Menu functions

///////////////////////////////////////////////////////////////////////////////
// Bitmap functions

#pragma region Bitmap functions

/// Create bitmap pointed to by `m_pBitmap` and set all pixels to white.
/// \param w Bitmap width in pixels.
/// \param h Bitmap height in pixels.

void CMain::CreateBitmap(int w, int h){
  delete m_pBitmap; //safety
  m_pBitmap = new Gdiplus::Bitmap(w, h); //create bitmap
  ClearBitmap(Gdiplus::Color::White); //clear bitmap to white
} //CreateBitmap

/// Clear the bitmap pointed to by `m_pBitmap`.
/// \param clr Color to set the pixels of `m_pBitmap` to.

void CMain::ClearBitmap(Gdiplus::Color clr){ 
  Gdiplus::Graphics graphics(m_pBitmap); //for editing
  graphics.Clear(clr); //clear to white
} //Clear

/// Set a grayscale pixel in the bitmap pointed to by `m_pBitmap` from a
/// floating point value in \f$[-1, 1]\f$, where \f$-1\f$ is black and \f$+1\f$
/// is white. The indices of the pixel are assumed to be in range.
/// \param i Row number.
/// \param j Column number.
/// \param g Grayscale value in the range \f$[-1, 1]\f$.

void CMain::SetPixel(UINT i, UINT j, float g){
  SetPixel(i, j, BYTE(float(0xFF)*(g/2 + 0.5f)));
} //SetPixel

/// Set a grayscale pixel in the bitmap pointed to by `m_pBitmap` from a byte
/// value in the range \f$[0, 255]\f$, where \f$0\f$ is black and \f$255\f$ is
/// white. The indices of the pixel are assumed to be in range.
/// \param i Row number.
/// \param j Column number.
/// \param b Grayscale value in the range \f$[0, 255]\f$.

void CMain::SetPixel(UINT i, UINT j, BYTE b){
  SetPixel(i, j, Gdiplus::Color(255, b, b, b));
} //SetPixel

/// Set a pixel in the bitmap pointed to by `m_pBitmap` to a GDI+ color. The
/// indices of the pixel are assumed to be in range.

void CMain::SetPixel(UINT i, UINT j, Gdiplus::Color clr){
  m_pBitmap->SetPixel(i, j, clr);
} //SetPixel

#pragma endregion Bitmap functions

///////////////////////////////////////////////////////////////////////////////
// Noise generation functions

#pragma region Noise generation functions

/// Generate Perlin or Value noise into the bitmap pointed to by `m_pBitmap`.
/// Pixel coordinates (which are whole numbers) are offset by `m_fOriginX`
/// and `m_fOriginY` and scaled by `m_fScale` to get noise coordinates (which
/// are floating point numbers).
/// \param t Type of noise.

void CMain::GenerateNoiseBitmap(eNoise t){ 
  m_eNoise = t; //remember the noise type
  UpdateMenus(); //changing noise type may change the menu status

  for(UINT i=0; i<m_pBitmap->GetWidth(); i++){
    const float x = m_fOriginX + i/m_fScale;

    for(UINT j=0; j<m_pBitmap->GetHeight(); j++){
      const float y = m_fOriginY + j/m_fScale;
      SetPixel(i, j, m_pPerlin->generate(x, y, t, m_nOctaves));
    } //for
  } //for
} //GenerateNoiseBitmap

/// Generate last type of noise.

void CMain::GenerateNoiseBitmap(){
  GenerateNoiseBitmap(m_eNoise);
} //GenerateNoiseBitmap

#pragma endregion Noise generation functions

///////////////////////////////////////////////////////////////////////////////
// Menu response functions

#pragma region Menu response functions

/// Set Perlin noise probability distribution and regenerate noise. This will
/// change the contents of the Perlin noise gradient/height table.
/// \param d Probability distribution enumerated type.

void CMain::SetDistribution(eDistribution d){
  m_eDistr = d;
  m_pPerlin->RandomizeTable(d);
  UpdateDistributionMenu(m_hDistMenu, m_eNoise, m_eDistr);
  GenerateNoiseBitmap();
} //SetDistribution

/// Set Perlin noise spline function and regenerate noise.
/// \param d Spline function enumerated type.

void CMain::SetSpline(eSpline d){
  m_eSpline = d;
  m_pPerlin->SetSpline(d);
  UpdateSplineMenu(m_hSplineMenu, m_eNoise, d);
  GenerateNoiseBitmap();
} //SetSpline

/// Set Perlin noise hash function and regenerate noise.
/// \param d Hash function enumerated type.

void CMain::SetHash(eHash d){
  m_eHash = d;
  m_pPerlin->SetHash(d);
  UpdateHashMenu(m_hHashMenu, m_eNoise, d);
  GenerateNoiseBitmap();
} //SetHash

/// Increment both coordinates of the origin by the table size and regenerate
/// noise.

void CMain::Jump(){
  const float offset = round(pow(2.0f, (float)m_nLog2TableSize));
  m_fOriginX += offset;
  m_fOriginY += offset;
  GenerateNoiseBitmap();
} //Jump

/// Set coordinates of the origin.
/// \param x New X-coordinate of origin.
/// \param y New Y-coordinate of origin.

void CMain::Jump(float x, float y){
  m_fOriginX = x;
  m_fOriginY = y;
  GenerateNoiseBitmap();
} //Jump

/// Check origin coordinates.
/// \param x Desired X-coordinate of origin.
/// \param y Desired Y-coordinate of origin.
/// \return true if the origin coordinates are the desired coordinates.

const bool CMain::Origin(float x, float y) const{
  return m_fOriginX == x && m_fOriginY == y;
} //Origin

/// Increase the number of octaves in `m_nOctaves` by one up to a maximum
/// of `m_nMaxOctaves`.

void CMain::IncreaseOctaves(){
  m_nOctaves = std::min<size_t>(m_nOctaves + 1, m_nMaxOctaves);
  GenerateNoiseBitmap();
} //IncreaseOctaves

/// Decrease the number of octaves in `m_nOctaves` by one down to a minimum
/// of `m_nMinOctaves`.

void CMain::DecreaseOctaves(){
  m_nOctaves = std::max<size_t>(m_nMinOctaves, m_nOctaves - 1);
  GenerateNoiseBitmap();
} //DecreaseOctaves

/// Increase the scale in `m_fScale` by a factor of 2 up to a maximum
/// of `m_fMaxScale`.

void CMain::IncreaseScale(){
  m_fScale = std::min<float>(2.0f*m_fScale, m_fMaxScale);
  GenerateNoiseBitmap();
} //IncreaseScale

/// Decrease the scale in `m_fScale` by a factor of 2 down to a minimum
/// of `m_fMinScale`.

void CMain::DecreaseScale(){
  m_fScale = std::max<float>(m_fMinScale, m_fScale/2.0f);
  GenerateNoiseBitmap();
} //DecreaseScale

/// Increase the table size by a factor of 2 by adding one to `m_nLog2TableSize`
/// up to a maximum of `m_nMaxLog2TableSize`.

void CMain::IncreaseTableSize(){
  m_nLog2TableSize = std::min<UINT>(m_nLog2TableSize + 1, m_nMaxLog2TableSize);
  delete m_pPerlin;
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize);
  GenerateNoiseBitmap();
} //IncreaseTableSize

/// Decrease the table size by a factor of 2 by subtracting one from
/// `m_nLog2TableSize` down to a minimum of `m_nMinLog2TableSize`.

void CMain::DecreaseTableSize(){
  m_nLog2TableSize = std::max<UINT>(m_nMinLog2TableSize, m_nLog2TableSize - 1);
  delete m_pPerlin;
  m_pPerlin = new CPerlinNoise2D(m_nLog2TableSize);
  GenerateNoiseBitmap();
} //DecreaseTableSize

/// Reset number of octaves, scale, and table size to defaults.

void CMain::Reset(){
  m_nOctaves = m_nDefOctaves;
  m_fScale = m_fDefScale;
  m_nLog2TableSize = m_nDefLog2TableSize;
  GenerateNoiseBitmap();
} //Reset

#pragma endregion Menu response functions

///////////////////////////////////////////////////////////////////////////////
// Reader functions

#pragma region Reader functions

/// Make up a file name from the noise parameters.
/// \param return A file name with no spaces and without extension.

const std::wstring CMain::GetFileName() const{
  std::wstring wstr;

  switch(m_eNoise){
    case eNoise::Perlin: wstr = L"Perlin"; break;
    case eNoise::Value:  wstr = L"Value";  break;
  } //switch

  switch(m_eHash){
    case eHash::Permutation: wstr += L"-Perm"; break;
    case eHash::Std:  wstr += L"-Std";  break;
  } //switch

  switch(m_eDistr){
    case eDistribution::Uniform: break; //nothing, which is the default  
    case eDistribution::Cosine:  wstr += L"-Cos"; break;   
    case eDistribution::Normal:  wstr += L"-Norm"; break;    
    case eDistribution::Exponential: wstr += L"-Exp"; break;
  } //switch

  switch(m_eSpline){
    case eSpline::None: wstr += L"-NoSpline"; break; 
    case eSpline::Cubic: break; //nothing, which is the default
    case eSpline::Quintic: wstr += L"-Quintic"; break;
  } //switch

  wstr += L"-" + std::to_wstring(m_nOctaves);
  wstr += L"-" + std::to_wstring(1 << m_nLog2TableSize);
  wstr += L"-" + std::to_wstring((size_t)round(m_fScale));

  return wstr;
} //GetFileName

/// Get noise description including type of noise and its parameters.
/// \return Wide string noise description.

const std::wstring CMain::GetNoiseDescription() const{
  std::wstring wstr;

  if(m_eNoise == eNoise::Perlin || m_eNoise == eNoise::Value){
    wstr += std::to_wstring(m_nOctaves) + L" octave";
    if(m_nOctaves > 1)wstr += L"s";
    wstr += L" of ";
  } //if

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L"Perlin"; break;
    case eNoise::Value:  wstr += L"Value";  break;
  } //switch
  
  wstr += L" Noise with origin (";
  wstr += to_wstring_f(m_fOriginX, 2) + L", ";
  wstr += to_wstring_f(m_fOriginY, 2) + L"), ";

  switch(m_eHash){
    case eHash::Permutation: wstr += L"a permutation"; break;
    case eHash::Std:  wstr += L"std";  break;
  } //switch

  wstr += L" hash function, ";

  switch(m_eDistr){
    case eDistribution::Uniform: wstr += L"uniform"; break;    
    case eDistribution::Cosine:  wstr += L"cosine"; break;   
    case eDistribution::Normal:  wstr += L"normal"; break;    
    case eDistribution::Exponential: wstr += L"exponential"; break;
  } //switch

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L" gradient"; break;
    case eNoise::Value:  wstr += L" height";  break;
  } //switch

  wstr += L" distribution, ";

  switch(m_eSpline){
    case eSpline::None:    wstr += L"no"; break; 
    case eSpline::Cubic:   wstr += L"cubic"; break; //nothing
    case eSpline::Quintic: wstr += L"quintic"; break;
  } //switch

  wstr += L" spline function, scale ";
  wstr += std::to_wstring((size_t)round(m_fScale));
  wstr += L", and ";
  
  if(m_eHash == eHash::Permutation)
    wstr += L"permutation and ";

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L"gradient "; break;
    case eNoise::Value:  wstr += L"height ";  break;
  } //switch
      
  wstr += L"table size ";
  wstr += std::to_wstring(1 << m_nLog2TableSize);
  wstr += L". Lacunarity and persistence are fixed at 0.5 and 2.0, respectively.";

  return wstr;
} //GetNoiseDescription

/// Reader function for the bitmap pointer `m_pBitmap`.
/// \return The bitmap pointer `m_pBitmap`.

Gdiplus::Bitmap* CMain::GetBitmap() const{
  return m_pBitmap;
} //GetBitmap

/// Reader function for the distribution type `m_eDistr`.
/// \return The distribution type `m_eDistr`.

const eDistribution CMain::GetDistribution() const{
  return m_eDistr;
} //GetDistribution

#pragma endregion Reader functions
