////////////////////////////////////////////////////////////////////////////////////////////////////
// OcrWithTesseract.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage OcrWithTesseract_cpp
*/
/*!
\page OcrWithTesseract_cpp Ocr With Tesseract Sample
Example how to convert an image based PDF to searchable document.
NOTE: If your tessdata dir is in the /usr/share/tesseract-ocr dir, data_path should be set to /usr/share/tesseract-ocr.
\snippet /OcrWithTesseract.hpp OcrWithTesseract_cpp
*/

#pragma once

//! [OcrWithTesseract_cpp]
#include <string>
#include <iostream>
#include "Pdfix.h"
#include "OcrTesseract.h"

#define kPi 3.1415926535897932384626433832795f

extern void PdfMatrixRotate(PdfMatrix& m, double radian, bool prepend);
extern void PdfMatrixScale(PdfMatrix& m, double sx, double sy, bool prepend);
extern void PdfMatrixTranslate(PdfMatrix& m, double x, double y, bool prepend);

void OcrWithTesseract(
  const std::wstring& email,                      // authorization email
  const std::wstring& license_key,                // authorization license key
  const std::wstring& open_path,                  // source PDF document
  const std::wstring& save_path,                  // searchable PDF document
  const std::wstring& data_path,                  // path to OCR data
  const std::wstring& language,                   // default OCR language
  const double zoom,                              // page zoom level for rendering to control image processing quality
  const PdfRotate rotate                          // page rotation
) {
  // initialize Pdfix
  if (!Pdfix_init(Pdfix_MODULE_NAME))
    throw std::runtime_error("Pdfix initialization fail");

  Pdfix* pdfix = GetPdfix();
  if (!pdfix)
    throw std::runtime_error("GetPdfix fail");
  if (!pdfix->Authorize(email.c_str(), license_key.c_str()))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  // initialize OcrTesseract
  if (!OcrTesseract_init(OcrTesseract_MODULE_NAME))
    throw std::runtime_error("OcrTesseract_init fail");

  OcrTesseract* ocr = GetOcrTesseract();
  if (!ocr)
    throw std::runtime_error("GetOcrTesseract fail");

  std::cout << "PDFix OCR Tesseract " << ocr->GetVersionMajor() << "." <<
    ocr->GetVersionMinor() << "." <<
    ocr->GetVersionPatch() << std::endl;

  if (!ocr->Initialize(pdfix))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  PdfDoc* doc = pdfix->OpenDoc(open_path.c_str(), L"");
  if (!doc)
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  ocr->SetLanguage(language.c_str());
  ocr->SetDataPath(data_path.c_str());

  TesseractDoc* ocr_doc = ocr->OpenOcrDoc(doc);
  if (!ocr_doc)
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  // ocr each page in the document
  for (int i = 0; i < doc->GetNumPages(); i++) {
    PdfPage* page = doc->AcquirePage(i);
    
    PdfRect crop_box;
    page->GetCropBox(&crop_box);
    
    PdfPageView* page_view = page->AcquirePageView(zoom, rotate);
    
    // draw page to an image
    PsImage* image = pdfix->CreateImage(page_view->GetDeviceWidth(), page_view->GetDeviceHeight(),
      kImageDIBFormatArgb);
    if (!image)
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    
    PdfPageRenderParams params;
    params.image = image;
    page_view->GetDeviceMatrix(&params.matrix);
    
    if (!page->DrawContent(&params, nullptr, nullptr))
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    
    // calculate PdfMatrix to position the recognized text on the page
    auto rotate = ((page->GetRotate() / 90) % 4);
    PdfMatrix matrix;
    PdfMatrixRotate(matrix, rotate * kPi / 2, false);
    PdfMatrixScale(matrix, 1/zoom, 1/zoom, false);
    switch (rotate) {
      case 0: PdfMatrixTranslate(matrix, crop_box.left, crop_box.bottom, false); break;
      case 1: PdfMatrixTranslate(matrix, crop_box.right, crop_box.bottom, false); break;
      case 2: PdfMatrixTranslate(matrix, crop_box.right, crop_box.top, false); break;
      case 3: PdfMatrixTranslate(matrix, crop_box.left, crop_box.top, false); break;
    }

    if (!ocr_doc->OcrImageToPage(image, &matrix, page, nullptr, nullptr))
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    
    page->Release();
  }
  
  if (!doc->Save(save_path.c_str(), kSaveFull))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  ocr_doc->Close();
  ocr->Destroy();

  doc->Close();
  pdfix->Destroy();
}
//! [OcrWithTesseract_cpp]
