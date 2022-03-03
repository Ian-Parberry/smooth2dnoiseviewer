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
  m_pPerlin = new CPerlinNoise2D(); //Perlin noise generator
  CreateMenus(); //create the menu bar
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

/// Draw the bitmap to the window client area, scaled down if necessary. This
/// function should only be called in response to a `WM_PAINT` message.

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
  m_hViewMenu = CreateViewMenu(hMenubar);
  m_hDistMenu = CreateDistributionMenu(hMenubar);
  m_hHashMenu = CreateHashMenu(hMenubar);
  m_hSplineMenu = CreateSplineMenu(hMenubar);
  m_hSetMenu = CreateSettingsMenu(hMenubar);
  CreateHelpMenu(hMenubar);

  SetMenu(m_hWnd, hMenubar);
  UpdateMenus();
} //CreateMenus

/// Update all menus, that is, gray out inactive menu items and place a 
/// checkmark next to chosen menu items.

void CMain::UpdateMenus(){
  UpdateFileMenu(m_hFileMenu, m_eNoise); 
  UpdateGenerateMenu(m_hGenMenu, m_eNoise);
  UpdateViewMenu(m_hViewMenu, m_eNoise);
  UpdateDistributionMenu(m_hDistMenu, m_eNoise, m_eDistr); 
  UpdateHashMenu(m_hHashMenu, m_eNoise, m_eHash); 
  UpdateSplineMenu(m_hSplineMenu, m_eNoise, m_eSpline); 
  UpdateSettingsMenu(m_hSetMenu, m_eNoise); 

  //update individual menu items

  UpdateMenuItemGray(m_hGenMenu, IDM_GENERATE_RESETORIGIN, m_eNoise,
    m_fOriginX == 0.0f && m_fOriginY == 0.0f);

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
    m_pPerlin->GetTableSize(), m_pPerlin->GetMinTableSize(), 
    m_pPerlin->GetMaxTableSize()); 
  UpdateMenuItemGray(m_hSetMenu, IDM_SETTINGS_RESET, m_eNoise,
    m_nOctaves == m_nDefOctaves && m_fScale == m_fDefScale
    && m_pPerlin->GetTableSize() == m_pPerlin->GetDefTableSize()); 
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

/// Clear the bitmap.
/// \param clr Color to set the pixels of `m_pBitmap` to.

void CMain::ClearBitmap(Gdiplus::Color clr){ 
  Gdiplus::Graphics graphics(m_pBitmap); //for editing
  graphics.Clear(clr); //clear to white
} //Clear

/// Set a grayscale pixel in the bitmap from a floating point value in 
/// \f$[-1, 1]\f$, where \f$-1\f$ is black and \f$+1\f$ is white. The indices
/// of the pixel are assumed to be in range.
/// \param i Row number.
/// \param j Column number.
/// \param g Grayscale value in the range \f$[-1, 1]\f$.

void CMain::SetPixel(UINT i, UINT j, float g){
  SetPixel(i, j, BYTE(float(0xFF)*(g/2 + 0.5f)));
} //SetPixel

/// Set a grayscale pixel in the bitmap from a byte value in the range
/// \f$[0, 255]\f$, where \f$0\f$ is black and \f$255\f$ is white. The indices
/// of the pixel are assumed to be in range.
/// \param i Row number.
/// \param j Column number.
/// \param b Grayscale value in the range \f$[0, 255]\f$.

void CMain::SetPixel(UINT i, UINT j, BYTE b){
  SetPixel(i, j, Gdiplus::Color(255, b, b, b));
} //SetPixel

/// Set a pixel in the bitmap to a GDI+ color. The indices of the pixel are 
/// assumed to be in range.

void CMain::SetPixel(UINT i, UINT j, Gdiplus::Color clr){
  m_pBitmap->SetPixel(i, j, clr);
} //SetPixel

#pragma endregion Bitmap functions

///////////////////////////////////////////////////////////////////////////////
// Noise generation functions

#pragma region Noise generation functions

/// Generate Perlin or Value noise into the bitmap. Pixel coordinates (which 
/// are whole numbers) are offset by `m_fOriginX` and `m_fOriginY` and scaled
/// by `m_fScale` to get noise coordinates (which are floating point numbers).
/// \param t Type of noise.

void CMain::GenerateNoiseBitmap(eNoise t){ 
  m_eNoise = t; //remember the noise type
  UpdateMenus(); //changing noise type may change the menu status

  m_fMin = 1000.0f; //init noise minima to something stupidly large
  m_fMax = -1000.0f; //init noise maxima to something stupidly small
  m_fAve = 0.0f; //init sum for average

  const UINT w = m_pBitmap->GetWidth(); //bitmap width
  const UINT h = m_pBitmap->GetHeight(); //bitmap height

  for(UINT i=0; i<w; i++){
    const float x = m_fOriginX + i/m_fScale; //noise X-coordinate

    for(UINT j=0; j<h; j++){
      const float y = m_fOriginY + j/m_fScale; //noise Y-coordinate
      const float noise = m_pPerlin->generate(x, y, t, m_nOctaves); //noise
      SetPixel(i, j, noise); //draw noise pixel to bitmap

      //recompute maximum, minimum, and sum (for average)
      m_fMin = min(m_fMin, noise);
      m_fMax = max(m_fMax, noise);
      m_fAve += noise;
    } //for
  } //for

  m_fAve /= w*h; //compute average from sum
  
  if(m_bShowGrid)DrawGrid();
  if(m_bShowCoords)DrawCoords();
} //GenerateNoiseBitmap

/// Generate Perlin or Value noise into a rectangle in the bitmap.
/// \param point The position of the rectangle to be written in the bitmap.
/// \param rect The bounds of the rectangle to be written in the bitmap. 

void CMain::GenerateNoiseBitmap(Gdiplus::PointF point, Gdiplus::RectF rect){
  const UINT nLeft   = (UINT)floorf(rect.GetLeft()  + point.X);
  const UINT nRight  = (UINT)ceilf(rect.GetRight()  + point.X);
  const UINT nTop    = (UINT)floorf(rect.GetTop()   + point.Y);
  const UINT nBottom = (UINT)ceilf(rect.GetBottom() + point.Y);

  for(UINT i=nLeft; i<nRight; i++){
    const float x = m_fOriginX + i/m_fScale;

    for(UINT j=nTop; j<nBottom; j++){
      const float y = m_fOriginY + j/m_fScale;
      SetPixel(i, j, m_pPerlin->generate(x, y, m_eNoise, m_nOctaves));
    } //for
  } //for
} //GenerateNoiseBitmap
 
/// If `m_bShowCoords` is `true`, then draw the coordinates of the top left
/// and bottom right of the noise to the corresponding corners of the bitmap.
/// The font family, font, style, and color of the text are hard-coded.
/// If `m_bShowCoords` is `false`, then overwrite the pixels within the
/// bounding rectangle of the text coordinates with the corresponding noise.

void CMain::DrawCoords(){ 
  Gdiplus::FontFamily ff(L"Arial");
  Gdiplus::Font font(&ff, 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
  Gdiplus::SolidBrush brush(Gdiplus::Color::White);

  Gdiplus::Graphics graphics(m_pBitmap);

  //top left corner coordinates

  Gdiplus::PointF point(0.0f, 0.0f); //text position on screen
  
  std::wstring wstr = L"("; //text of origin coordinates
  wstr += std::to_wstring((size_t)floorf(m_fOriginX)) + L", ";
  wstr += std::to_wstring((size_t)floorf(m_fOriginY)) + L")";
  
  Gdiplus::RectF rect, unused;
  graphics.MeasureString(wstr.c_str(), -1, &font, unused, &rect); //measure text

  if(m_bShowCoords) //draw
    graphics.DrawString(wstr.c_str(), -1, &font, point, &brush);
  else GenerateNoiseBitmap(point, rect); //undraw

  //bottom right corner coordinates

  const float x = m_fOriginX + m_pBitmap->GetWidth()/m_fScale; //x-coordinate
  const float y = m_fOriginY + m_pBitmap->GetHeight()/m_fScale; //y-coordinate

  wstr = L"(" + to_wstring_f(x, 2) + L", " + to_wstring_f(y, 2) + L")"; //text

  graphics.MeasureString(wstr.c_str(), -1, &font, unused, &rect); //measure text
  point.X = m_pBitmap->GetWidth()  - rect.Width; //text x-coordinate on screen
  point.Y = m_pBitmap->GetHeight() - rect.Height; //text y-coordinate on screen
  
  if(m_bShowCoords) //draw
    graphics.DrawString(wstr.c_str(), -1, &font, point, &brush);
  else GenerateNoiseBitmap(point, rect); //undraw

  if(m_bShowGrid && !m_bShowCoords)
    DrawGrid(); //in case we erased part of the grid lines
} //DrawCoords
 
/// Erase the coordinates of the top left and bottom right of the noise from
/// the bitmap pointed to by `m_pBitmap` by overdrawing the corresponding
/// rectangles with Perlin noise.

void CMain::UndrawCoords(){ 
  //TODO: this. Will need a function to overdraw a rectangle too.
} //UndrawCoords

/// If `m_bShowGrid` is `true`, draw a grid for first noise octave to the
/// bitmap. The line color for the grid is fixed. If `m_bShowGrid` is `false`,
/// then overwrite the pixels in the lines with the corresponding noise.

void CMain::DrawGrid(){ 
  const float fBitmapWidth  = (float)m_pBitmap->GetWidth();
  const float fBitmapHeight = (float)m_pBitmap->GetHeight();

  const UINT nv = (UINT)floorf(fBitmapWidth/m_fScale); //number of vertical lines
  const UINT nh = (UINT)floorf(fBitmapHeight/m_fScale); //number of horizontal lines

  Gdiplus::Graphics graphics(m_pBitmap);
  Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0)); //green

  //horizontal lines

  Gdiplus::PointF left(0.0f, m_fScale); //left X-coordinate of line
  Gdiplus::PointF right(fBitmapWidth, m_fScale); //right X-coordinate of line
  Gdiplus::RectF rect(0.0f, 0.0f, fBitmapWidth, 1.0f); //bounding rectangle

  for(UINT i=0; i<nh; i++){ //for each horizontal line
    if(m_bShowGrid)graphics.DrawLine(&pen, left, right); //draw line
    else GenerateNoiseBitmap(left, rect); //undraw line

    //move down to next horizontal line
    left.Y += m_fScale; 
    right.Y += m_fScale;
  } //for

  //vertical lines

  Gdiplus::PointF top(m_fScale, 0.0f); //top Y-coordinate of line
  Gdiplus::PointF bottom(m_fScale, fBitmapHeight); //bottom Y-coordinate of line
  rect = Gdiplus::RectF(0.0f, 0.0f, 1.0f, fBitmapHeight); //bounding rectangle

  for(UINT i=0; i<nv; i++){ //for each vertical line
    if(m_bShowGrid)graphics.DrawLine(&pen, top, bottom); //draw line
    else GenerateNoiseBitmap(top, rect); //undraw line
    
    //move right to next vertical line
    top.X += m_fScale;
    bottom.X += m_fScale;
  } //for

  if(!m_bShowGrid && m_bShowCoords)
    DrawCoords(); //in case we erased some of the coordinate text
} //DrawGrid

/// Generate last type of noise.

void CMain::GenerateNoiseBitmap(){
  GenerateNoiseBitmap(m_eNoise);
} //GenerateNoiseBitmap

#pragma endregion Noise generation functions

///////////////////////////////////////////////////////////////////////////////
// Menu response functions

#pragma region Menu response functions

void CMain::Randomize(){
  m_pPerlin->Randomize();
  UpdateDistribution();
} //Randomize

/// Change Perlin noise probability distribution and regenerate noise. 
/// This will have no effect if the new distribution is the same as the old one.
/// \param d Probability distribution enumerated type.
/// \return true if the distribution changed.

bool CMain::SetDistribution(eDistribution d){
  if(m_eDistr != d){
    m_eDistr = d;
    UpdateDistribution();
    return true;
  } //if

  return false;
} //SetDistribution

/// Update Perlin noise probability distribution and regenerate noise.

void CMain::UpdateDistribution(){
  m_pPerlin->RandomizeTable(m_eDistr);
  UpdateDistributionMenu(m_hDistMenu, m_eNoise, m_eDistr);
  GenerateNoiseBitmap();
} //UpdateDistribution

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

/// Toggle the View Coordinates flag, put a checkmark next to the menu item,
/// and redraw the bitmap.

void CMain::ToggleViewCoords(){
  m_bShowCoords = !m_bShowCoords;
  UpdateMenuItemCheck(m_hViewMenu, IDM_VIEW_COORDS, m_bShowCoords);
  DrawCoords();
} //ToggleViewCoords

/// Toggle the View Grid flag, put a checkmark next to the menu item, and
/// redraw the bitmap.

void CMain::ToggleViewGrid(){
  m_bShowGrid = !m_bShowGrid;
  UpdateMenuItemCheck(m_hViewMenu, IDM_VIEW_GRID, m_bShowGrid);
  DrawGrid();
} // ToggleViewGrid

/// Increment both coordinates of the origin by `m_nTableSize` and regenerate
/// noise.

void CMain::Jump(){
  const float offset = (float)m_pPerlin->GetTableSize();
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

/// Increase the table size by a factor of 2.

void CMain::IncreaseTableSize(){
  if(m_pPerlin->DoubleTableSize())
    GenerateNoiseBitmap();
} //IncreaseTableSize

/// Decrease the table size by a factor of 2 `.

void CMain::DecreaseTableSize(){
  if(m_pPerlin->HalveTableSize())
    GenerateNoiseBitmap();
} //DecreaseTableSize

/// Reset number of octaves, scale, and table size to defaults.

void CMain::Reset(){
  m_nOctaves = m_nDefOctaves;
  m_fScale = m_fDefScale;
  m_pPerlin->DefaultTableSize();
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
    case eHash::Permutation:        wstr += L"-Perm"; break;
    case eHash::LinearCongruential: wstr += L"-Lin";  break;
    case eHash::Std:                wstr += L"-Std";  break;
  } //switch

  switch(m_eDistr){
    case eDistribution::Uniform: break; //nothing, which is the default  
    case eDistribution::Maximal:     wstr += L"-Max";  break;    
    case eDistribution::Cosine:      wstr += L"-Cos";  break;   
    case eDistribution::Normal:      wstr += L"-Norm"; break;    
    case eDistribution::Exponential: wstr += L"-Exp";  break;    
    case eDistribution::Midpoint:    wstr += L"-Mid";  break;
  } //switch

  switch(m_eSpline){
    case eSpline::None: wstr += L"-NoSpline"; break; 
    case eSpline::Cubic: break; //nothing, which is the default
    case eSpline::Quintic: wstr += L"-Quintic"; break;
  } //switch

  wstr += L"-" + std::to_wstring(m_nOctaves);
  wstr += L"-" + std::to_wstring(m_pPerlin->GetTableSize());
  wstr += L"-" + std::to_wstring((size_t)round(m_fScale));

  return wstr;
} //GetFileName

/// Get noise description including type of noise and its parameters.
/// \return Wide string noise description.

const std::wstring CMain::GetNoiseDescription() const{
  std::wstring wstr; //for noise description

  //number of octaves

  if(m_eNoise == eNoise::Perlin || m_eNoise == eNoise::Value){
    wstr += std::to_wstring(m_nOctaves) + L" octave";
    if(m_nOctaves > 1)wstr += L"s";
    wstr += L" of ";
  } //if

  //type of noise

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L"Perlin"; break;
    case eNoise::Value:  wstr += L"Value";  break;
  } //switch

  wstr += L" Noise";

  //origin
  
  wstr += L" with origin (";
  wstr += to_wstring_f(m_fOriginX, 2) + L", ";
  wstr += to_wstring_f(m_fOriginY, 2) + L"), ";

  //hash function

  switch(m_eHash){
    case eHash::Permutation:         wstr += L"a permutation"; break;
    case eHash::LinearCongruential:  wstr += L"linear congruential"; break;
    case eHash::Std:                 wstr += L"std";  break;
  } //switch

  wstr += L" hash function, ";

  //distribution

  switch(m_eDistr){
    case eDistribution::Uniform:     wstr += L"uniform"; break;  
    case eDistribution::Maximal:     wstr += L"maximal"; break;    
    case eDistribution::Cosine:      wstr += L"cosine"; break;   
    case eDistribution::Normal:      wstr += L"normal"; break;    
    case eDistribution::Exponential: wstr += L"exponential"; break;
    case eDistribution::Midpoint:    wstr += L"midpoint displacement"; break;
  } //switch

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L" gradient"; break;
    case eNoise::Value:  wstr += L" height";   break;
  } //switch

  wstr += L" distribution, ";

  //spline function

  switch(m_eSpline){
    case eSpline::None:    wstr += L"no";      break; 
    case eSpline::Cubic:   wstr += L"cubic";   break; 
    case eSpline::Quintic: wstr += L"quintic"; break;
  } //switch

  wstr += L" spline function, ";

  //scale

  wstr += L"scale " + std::to_wstring((size_t)round(m_fScale)) + L", and ";

  //table size
  
  if(m_eHash == eHash::Permutation)
    wstr += L"permutation and ";

  switch(m_eNoise){
    case eNoise::Perlin: wstr += L"gradient "; break;
    case eNoise::Value:  wstr += L"value ";  break;
  } //switch
      
  wstr += L"table size ";
  wstr += std::to_wstring(m_pPerlin->GetTableSize());
  wstr += L". ";

  //noise max, min, and average

  wstr += L"Largest generated noise ";
  wstr += to_wstring_f(m_fMax, 4) + L". ";
  wstr += L"Smallest generated noise ";
  wstr += to_wstring_f(m_fMin, 4) + L". ";
  wstr += L"Average generated noise ";
  wstr += to_wstring_f(m_fAve, 4) + L".";

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
