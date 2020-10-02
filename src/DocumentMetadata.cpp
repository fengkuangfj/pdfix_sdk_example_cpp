////////////////////////////////////////////////////////////////////////////////////////////////////
// DocumentMetadata.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage DocumentMetadata_cpp
*/
/*!
\page DocumentMetadata_cpp Get/Set Document Metadata Sample
Example how to get or set document metadata.
\snippet /DocumentMetadata.hpp DocumentMetadata_cpp
*/

#include "pdfixsdksamples/DocumentMetadata.h"

//! [DocumentMetadata_cpp]
#include <string>
#include <iostream>
#include "Pdfix.h"

using namespace PDFixSDK;

void DocumentMetadata(
  const std::wstring& open_path,                       // source PDF document
  const std::wstring& save_path,                       // output PDF doucment
  const std::wstring& xml_path                         // metadata file path
) {
  // initialize Pdfix
  if (!Pdfix_init(Pdfix_MODULE_NAME))
    throw std::runtime_error("Pdfix initialization fail");

  Pdfix* pdfix = GetPdfix();
  if (!pdfix)
    throw std::runtime_error("GetPdfix fail");

  PdfDoc* doc = nullptr;
  doc = pdfix->OpenDoc(open_path.c_str(), L"");
  if (!doc)
    throw PdfixException();

  std::wstring title = doc->GetInfo(L"Title");
  doc->SetInfo(L"Title", L"My next presentation");

  PsMetadata* metadata = doc->GetMetadata();
  if (!metadata)
    throw PdfixException();

  PsFileStream* stream = pdfix->CreateFileStream(xml_path.c_str(), kPsTruncate);
  if (!metadata->SaveToStream(stream))
    throw PdfixException();
  stream->Destroy();

  // do something with XML metadata ...

  stream = pdfix->CreateFileStream(xml_path.c_str(), kPsReadOnly);
  if (!metadata->LoadFromStream(stream))
    throw PdfixException();
  stream->Destroy();

  if (!doc->Save(save_path.c_str(), kSaveFull))
    throw PdfixException();

  doc->Close();
  pdfix->Destroy();
}

//! [DocumentMetadata_cpp]
