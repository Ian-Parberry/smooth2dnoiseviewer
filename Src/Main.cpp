/// \file Main.cpp
/// \brief The window procedure WndProc(), and wWinMain().

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
#include "resource.h"

static CMain* g_pMain = nullptr; ///< Pointer to the main class.

static const int g_nWidth = 600; ///< Client area width in pixels.
static const int g_nHeight = 600; ///< Client area height in pixels.

/// \brief Window procedure.
///
/// This is the handler for messages from the operating system.
/// \param hWnd Window handle.
/// \param message Message code.
/// \param wParam Parameter for message.
/// \param lParam Second parameter for message if needed.
/// \return 0 If message is handled.

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  UINT nMenuId = 0; //menu identifier for menu command messages
  static bool bResizing = false;

  switch(message){
    case WM_CREATE: //window has been created
      g_pMain = new CMain(hWnd); //create the main class
      g_pMain->CreateBitmap(g_nWidth, g_nHeight);
      return 0;

    case WM_DESTROY: //window has been removed from the screen
      delete g_pMain; //delete the main class
      PostQuitMessage(0); //ready to shut down
      return 0;

    case WM_PAINT: //window needs to be redrawn
      g_pMain->OnPaint();
      return 0;
 
    case WM_COMMAND: //user has selected a command from the menu
      nMenuId = LOWORD(wParam); //menu id

      switch(nMenuId){  
        case IDM_SETTINGS_OCTAVE_UP:
          g_pMain->IncreaseOctaves();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SETTINGS_OCTAVE_DN:
          g_pMain->DecreaseOctaves();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SETTINGS_SCALE_UP:
          g_pMain->IncreaseScale();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SETTINGS_SCALE_DN:
          g_pMain->DecreaseScale();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SETTINGS_TSIZE_UP:
          g_pMain->IncreaseTableSize();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SETTINGS_TSIZE_DN:
          g_pMain->DecreaseTableSize();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_GENERATE_PERLINNOISE:
          g_pMain->GeneratePerlinNoise(eNoise::Perlin);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_GENERATE_PIXELNOISE:
          g_pMain->GeneratePixelNoise();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_GENERATE_VALUENOISE:
          g_pMain->GeneratePerlinNoise(eNoise::Value);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_GENERATE_RANDOMIZE:
          g_pMain->Randomize();
          InvalidateRect(hWnd, nullptr, FALSE);
          break;
            
        case IDM_DISTRIBUTION_UNIFORM:
          g_pMain->Initialize(eDistribution::Uniform);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_DISTRIBUTION_COSINE:
          g_pMain->Initialize(eDistribution::Cosine);
          InvalidateRect(hWnd, nullptr, FALSE);
          break; 

        case IDM_DISTRIBUTION_NORMAL:
          g_pMain->Initialize(eDistribution::Normal);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_DISTRIBUTION_EXPONENTIAL:
          g_pMain->Initialize(eDistribution::Exponential);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SPLINE_NONE:
          g_pMain->SetSpline(eSpline::None);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SPLINE_CUBIC:
          g_pMain->SetSpline(eSpline::Cubic);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_SPLINE_QUINTIC:
          g_pMain->SetSpline(eSpline::Quintic);
          InvalidateRect(hWnd, nullptr, FALSE);
          break;

        case IDM_FILE_SAVE: //save bitmap to image file       
          SaveBitmap(hWnd, g_pMain->GetFileName(), g_pMain->GetBitmap());
          break;

        case IDM_FILE_QUIT: //so long, farewell, auf weidersehn, goodbye!
          SendMessage(hWnd, WM_CLOSE, 0, 0);
          break;
      } //switch

      return 0; //all is good

    default: 
      return DefWindowProc(hWnd, message, wParam, lParam); //not my message
  } //switch
} //WndProc

/// Create and initialize a window.
/// \param hInst Instance handle.
/// \param nShow 1 to show window, 0 to hide.
/// \param WndProc Window procedure.

void InitWindow(HINSTANCE hInst, INT nShow, WNDPROC WndProc){
  const LPCWSTR appname = L"2D Noise Generator";
   
  WNDCLASSEX wndClass = {0}; //extended window class structure

  wndClass.cbSize         = sizeof(WNDCLASSEX);
  wndClass.style          = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc    = WndProc;
  wndClass.cbClsExtra     = 0;
  wndClass.cbWndExtra     = 0;
  wndClass.hInstance      = hInst;
  wndClass.hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
  wndClass.hCursor        = LoadCursor(nullptr, IDC_ARROW);
  wndClass.hbrBackground  = nullptr;
  wndClass.lpszMenuName   = nullptr;
  wndClass.lpszClassName  = appname;
   
  RegisterClassEx(&wndClass);

  const DWORD dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU; 
  const DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME;
    
  RECT r{0};  
  r.right = g_nWidth; 
  r.bottom = g_nHeight + GetSystemMetrics(SM_CYMENU);
  AdjustWindowRectEx(&r, dwStyle, FALSE, dwStyleEx); 

  const HWND hwnd = CreateWindowEx(dwStyleEx, appname, appname, dwStyle, 
    CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, 
    nullptr, nullptr, hInst, nullptr);
  
  ShowWindow(hwnd, nShow);
  UpdateWindow(hwnd);
} //InitWindow

/// \brief Winmain.  
///
/// Initialize a window and start the message pump. 
/// \param hInst Handle to the current instance.
/// \param hPrev Unused.
/// \param lpStr Unused.
/// \param nShow Nonzero if window is to be shown.
/// \return 0 If this application terminates correctly, otherwise an error code.

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpStr, int nShow){
  UNREFERENCED_PARAMETER(hPrev); //nope
  UNREFERENCED_PARAMETER(lpStr); //nope nope

  InitWindow(hInst, nShow, WndProc); //create and show a window

  MSG msg; //current message

  while(GetMessage(&msg, nullptr, 0, 0)){ //message pump
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  } //while

  return (int)msg.wParam;
} //wWinMain
