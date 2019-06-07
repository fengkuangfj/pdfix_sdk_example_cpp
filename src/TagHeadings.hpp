////////////////////////////////////////////////////////////////////////////////////////////////////
// TagHeadings.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage TagHeadings_cpp
*/
/*!
\page TagHeadings_cpp Tag paragraph as Heading
Example how to manually edit tags tagged P struct element to H1 based on the text prperties.
\snippet /TagHeadings.hpp TagHeadings_cpp
*/

#pragma once

//! [TagHeadings_cpp]
#include <string>
#include <iostream>
#include "Pdfix.h"

namespace TagHeadings {

//////////////////////////////////////////////////////////////////////////////////////////////////
// GetPageObjectTextState
// get text object's of specified mcid the text state
//////////////////////////////////////////////////////////////////////////////////////////////////
bool GetPageObjectTextState(PdsPageObject* page_object, int mcid, PdfTextState* ts) {
  if (page_object->GetObjectType() == kPdsPageText) {
    PdsText* text = (PdsText*)page_object;

    // check if this text page object has the same mcid
    PdsContentMark* content_mark = page_object->GetContentMark();
    if (content_mark && content_mark->GetTagMcid() == mcid) {
      text->GetTextState(page_object->GetPage()->GetDoc(), ts);
      return true;
    }
  }
  else if (page_object->GetObjectType() == kPdsPageForm) {
    // search for the text object inside of the form XObject
    PdsForm* form = (PdsForm*)page_object;
    for (int i = 0; i < form->GetNumPageObjects(); i++) {
      if (GetPageObjectTextState(form->GetPageObject(i), mcid, ts))
        return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// GetParagraphTextState
// get the text state of the text objects inside paragraph by iterating content kid objects
//////////////////////////////////////////////////////////////////////////////////////////////////
bool GetParagraphTextState(PdsStructElement* struct_elem, PdfTextState* ts) {
  for (int i = 0; i < struct_elem->GetNumKids(); i++) {
    if (struct_elem->GetKidType(i) == kPdsStructKidPageContent) {
      // acquire page on which the element is present
      PdfDoc* doc = struct_elem->GetStructTree()->GetDoc();
      auto page_deleter = [](PdfPage* page) { page->Release(); };
      std::unique_ptr<PdfPage, decltype(page_deleter)>
        page(doc->AcquirePage(struct_elem->GetKidPageNumber(i)), page_deleter);
      
      // find text object with mcid on the page to get the text state
      int mcid = struct_elem->GetKidMcid(i);
      for (int j = 0; j < page->GetNumPageObjects(); j++) {
        if (GetPageObjectTextState(page->GetPageObject(j), mcid, ts))
          return true;
      }
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// TagParagraphAsHeading
// re-tag the struct element to heading based on font properties
//////////////////////////////////////////////////////////////////////////////////////////////////
void TagParagraphAsHeading(PdsStructElement* struct_elem) {
  std::wstring type;
  type.resize(struct_elem->GetType(true, nullptr, 0));
  struct_elem->GetType(true, (wchar_t*)type.c_str(), (int)type.size());
  if (type == L"P") {
    // get the paragraph text_state
    PdfTextState ts;
    if (GetParagraphTextState(struct_elem, &ts) ) {
      // get the font name
      std::wstring font_name;
      font_name.resize(ts.font->GetFontName(nullptr, 0));
      ts.font->GetFontName((wchar_t*)font_name.c_str(), (int)font_name.size());
      
      std::wstring tag_type;
      if (font_name.find(L"Black") != std::wstring::npos && ts.font_size >= 25)
        tag_type = L"H1";
      else if ( font_name.find(L"Bold") != std::wstring::npos && ts.font_size >= 16)
        tag_type = L"H2";
      
      // update tag type
      if (!tag_type.empty()) {
        struct_elem->SetType(tag_type.c_str());
      }
    }
    return; // this was a P tag, no need to continue to kid struct elements
  }
  // search kid struct elements
  for (int i = 0; i < struct_elem->GetNumKids(); i++) {
    if (struct_elem->GetKidType(i) == kPdsStructKidElement) {
      PdsObject* kid_obj = struct_elem->GetKidObject(i);
      auto elem_deleter = [](PdsStructElement* elem) { elem->Release(); };
      std::unique_ptr<PdsStructElement, decltype(elem_deleter)>
      kid_elem(struct_elem->GetStructTree()->AcquireStructElement(kid_obj), elem_deleter);
      TagParagraphAsHeading(kid_elem.get());
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// TagParagraphAsHeading
// re-tag P tags to Hx tag based on predefined font properties (size, font name)
//////////////////////////////////////////////////////////////////////////////////////////////////
void Run(
  const std::wstring& email,            // authorization email
  const std::wstring& license_key,      // authorization license key
  const std::wstring& open_path,        // source PDF document
  const std::wstring& save_path         // output PDF document
) {
  // initialize Pdfix
  if (!Pdfix_init(Pdfix_MODULE_NAME))
    throw std::runtime_error("Pdfix initialization fail");

  Pdfix* pdfix = GetPdfix();
  if (!pdfix)
    throw std::runtime_error("GetPdfix fail");
  if (!pdfix->Authorize(email.c_str(), license_key.c_str()))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  PdfDoc* doc = pdfix->OpenDoc(open_path.c_str(), L"");
  if (!doc)
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  // cleanup any previous structure tree
  if (!doc->RemoveTags(nullptr, nullptr))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  // autotag document first
  if (!doc->AddTags(nullptr, nullptr))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  // get the struct tree
  auto struct_tree = doc->GetStructTree();
  if (!struct_tree)
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  // tag headings
  for (int i = 0; i < struct_tree->GetNumKids(); i++) {
    PdsObject* kid_obj = struct_tree->GetKidObject(i);
    auto elem_deleter = [](PdsStructElement* elem) { elem->Release(); };
    std::unique_ptr<PdsStructElement, decltype(elem_deleter)>
      kid_elem(struct_tree->AcquireStructElement(kid_obj), elem_deleter);
    TagParagraphAsHeading(kid_elem.get());
  }
  
  // save document
  if (!doc->Save(save_path.c_str(), kSaveFull))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  doc->Close();
  pdfix->Destroy();
}
  
} // namespace TagHeadings
//! [TagHeadings_cpp]
