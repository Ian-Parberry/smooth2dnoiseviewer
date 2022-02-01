/// \file WindowsHelpers.cpp
/// \brief Code for some helpful Windows-specific functions.
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

#pragma region Save

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
/// \param pBitmap Pointer to a bitmap.
/// \return S_OK for success, E_FAIL for failure.

HRESULT SaveBitmap(HWND hwnd, Gdiplus::Bitmap* pBitmap){
  COMDLG_FILTERSPEC filetypes[] = { //png files only
    {L"PNG Files", L"*.png"}
  }; //filetypes

  std::wstring wstrFileName; //result
  CComPtr<IFileSaveDialog> pDlg; //pointer to save dialog box
  static int n = 0; //number of images saved in this run
  std::wstring wstrName = L"Image" + std::to_wstring(n++); //default file name
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

#pragma endregion Save
