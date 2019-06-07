////////////////////////////////////////////////////////////////////////////////////////////////////
// DocumentMetadata.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage EmbedFonts_cpp
*/
/*!
\page EmbedFonts_cpp EmbedFonts Sample
Example how to Embed Fonts.
\snippet /EmbedFonts.hpp EmbedFonts_cpp
*/

#pragma once

//! [EmbedFonts_cpp]
#include <string>
#include <iostream>
#include "Pdfix.h"

void EmbedFonts(
  const std::wstring& email,                 // authorization email   
  const std::wstring& license_key,           // authorization license key
  const std::wstring& open_path,             // source PDF document
  const std::wstring& save_path              // output PDF doucment
) {
  // initialize Pdfix
  if (!Pdfix_init(Pdfix_MODULE_NAME))
    throw std::runtime_error("Pdfix initialization fail");

  Pdfix* pdfix = GetPdfix();
  if (!pdfix)
    throw std::runtime_error("GetPdfix fail");
  if (!pdfix->Authorize(email.c_str(), license_key.c_str()))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  PdfDoc* doc = nullptr;
  doc = pdfix->OpenDoc(open_path.c_str(), L"");
  if (!doc)
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  if (!doc->EmbedFonts(true, nullptr, nullptr))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  if (!doc->Save(save_path.c_str(), kSaveFull))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  doc->Close();
  pdfix->Destroy();
}
//! [EmbedFonts_cpp]
