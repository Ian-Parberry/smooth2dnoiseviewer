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

#include "CMain.h"
#include "WindowsHelpers.h"
#include "perlin.h"

#pragma comment(lib,"Winmm.lib")

///////////////////////////////////////////////////////////////////////////////
// Constructors and destructors

#pragma region Constructors and destructors

/// Initialize GDI+, create the menus, create a white bitmap.
/// \param hwnd Window handle.

CMain::CMain(const HWND hwnd): m_hWnd(hwnd){
  srand(timeGetTime()); //seed the PRNG

  m_gdiplusToken = InitGDIPlus(); //initialize GDI+
  CreateMenus(); //create the menu bar
} //constructor

/// Delete GDI+ objects, then shut down GDI+.

CMain::~CMain(){
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
  HMENU hMenu = CreateMenu();
  
  AppendMenuW(hMenu, MF_STRING, IDM_FILE_SAVE,     L"Save...");
  AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT,     L"Quit");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&File");
  
  hMenu = CreateMenu();

  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_PIXELNOISE,   L"Pixel noise");
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_PERLINNOISE,  L"Perlin noise");
  AppendMenuW(hMenu, MF_STRING, IDM_GENERATE_VALUENOISE,   L"Value noise");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&Generate");

  SetMenu(m_hWnd, hMenubar);
} //CreateMenus

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

/// Generate pixel noise into the bitmap pointed to by `m_pBitmap`. The
/// lower byte returned by the C standard function `rand()` is used as
/// a grayscale value for each pixel. This function generates a new pattern
/// each time it is called.

void CMain::GeneratePixelNoise(){
  for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
    for(UINT j=0; j<m_pBitmap->GetHeight(); j++){
      const BYTE gray = (BYTE)rand();
      m_pBitmap->SetPixel(i, j, Gdiplus::Color(255, gray, gray, gray));
    } //for
} //GeneratePixelNoise

/// Generate Perlin noise into the bitmap pointed to by `m_pBitmap`. 
/// This function generates a new pattern each time it is called.
/// For now, the scale, lacunarity, persistence, and number of octaves
/// is fixed.

void CMain::GeneratePerlinNoise(){
  initPerlin();
  const float fScale = 64.0f;

  for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
    for(UINT j=0; j<m_pBitmap->GetHeight(); j++){
      const float fPerlin = PerlinNoise2D(i/fScale, j/fScale, 0.5f, 2.0f, 4);
      const BYTE gray = BYTE(float(0xFF)*(fPerlin/2.0f + 0.5f));
      m_pBitmap->SetPixel(i, j, Gdiplus::Color(255, gray, gray, gray));
    } //for
} //GeneratePerlinNoise

/// Generate value noise into the bitmap pointed to by `m_pBitmap`. 
/// This function generates a new pattern each time it is called.
/// For now, the scale, lacunarity, persistence, and number of octaves
/// is fixed.

void CMain::GenerateValueNoise(){
  initPerlin();
  const float fScale = 64.0f;

  for(UINT i=0; i<m_pBitmap->GetWidth(); i++)
    for(UINT j=0; j<m_pBitmap->GetHeight(); j++){
      const float fPerlin = ValueNoise2D(i/fScale, j/fScale, 0.5f, 2.0f, 4);
      const BYTE gray = BYTE(float(0xFF)*(fPerlin/2.0f + 0.5f));
      m_pBitmap->SetPixel(i, j, Gdiplus::Color(255, gray, gray, gray));
    } //for
} //GeneratePerlinNoise

/// Reader function for the bitmap pointer `m_pBitmap`.
/// \return The bitmap pointer `m_pBitmap`.

Gdiplus::Bitmap* CMain::GetBitmap(){
  return m_pBitmap;
} //GetBitmap

