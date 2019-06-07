////////////////////////////////////////////////////////////////////////////////////////////////////
// TagsEditStructTree.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage TagsEditStructTree_cpp
*/
/*!
\page TagsEditStructTree_cpp Edit Structure Tree Sample
Example how to manually edit tags in a structur tree.
\snippet /TagsEditStructTree.hpp TagsEditStructTree_cpp
*/

#pragma once

//! [TagsEditStructTree_cpp]
#include <string>
#include <iostream>
#include "Pdfix.h"

auto struct_elem_deleter = [](PdsStructElement* elem) { elem->Release(); };
typedef std::unique_ptr<PdsStructElement, decltype(struct_elem_deleter)> PdsStructElementPtr;

//////////////////////////////////////////////////////////////////////
// Find struct element by the name in the structure element
//////////////////////////////////////////////////////////////////////
PdsStructElementPtr FindStructElement(PdsStructElement* struct_elem, const std::wstring& type,
  const std::wstring& title = L"") {
  // find object inside of the struct element with specified name
  auto struct_tree = struct_elem->GetStructTree();
  auto num_kids = struct_elem->GetNumKids();
  for (int i = 0; i < num_kids; i++) {
    auto kid_type = struct_elem->GetKidType(i);
    if (kid_type != kPdsStructKidElement)
      continue;
    PdsObject* kid_obj = struct_elem->GetKidObject(i);
    if (!kid_obj)
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    
    PdsStructElementPtr kid_elem(struct_tree->AcquireStructElement(kid_obj), struct_elem_deleter);

    // get struct element type and title
    std::wstring type_str(type), title_str(title);
    kid_elem->GetType(true, (wchar_t*)type_str.c_str(), (int)type_str.length());
    kid_elem->GetTitle((wchar_t*)title_str.c_str(), (int)title_str.length());

    // compare type and title
    if (type_str == type && (title.empty() || title_str == title))
      return kid_elem;
    
    // find in kid's elements
    if (auto found = FindStructElement(kid_elem.get(), type, title))
      return found;
  }
  return PdsStructElementPtr(nullptr, struct_elem_deleter);
}

//////////////////////////////////////////////////////////////////////
// Find struct element by the name in the structure tree
//////////////////////////////////////////////////////////////////////
PdsStructElementPtr FindStructElement(PdsStructTree* struct_tree, const std::wstring& name,
  const std::wstring& title = L"") {
  // find object inside of the struct tree
  for (int i = 0; i < struct_tree->GetNumKids(); i++) {
    auto kid_obj = struct_tree->GetKidObject(i);
    if (!kid_obj)
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    
    PdsStructElementPtr kid_elem(struct_tree->AcquireStructElement(kid_obj), struct_elem_deleter);
    if (!kid_elem)
      throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
    if (auto found = FindStructElement(kid_elem.get(), name, title))
      return found;
  }
  return PdsStructElementPtr(nullptr, struct_elem_deleter);
}

//////////////////////////////////////////////////////////////////////
// Change the tag name of the first table row from <td> to <th>
//////////////////////////////////////////////////////////////////////
void TableTagRowHeader(PdsStructElement* table) {
  auto struct_tree = table->GetStructTree();
  // get the first row td elements
  auto tr = FindStructElement(table, L"TR");
  for (int i = 0; i < tr->GetNumKids(); i++) {
    if (tr->GetKidType(i) == kPdsStructKidElement) {
      PdsObject* td_obj = tr->GetKidObject(i);
      if (!td_obj)
        throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
      
      PdsStructElementPtr td(struct_tree->AcquireStructElement(td_obj), struct_elem_deleter);
      
      std::wstring type(L"TD");
      td->GetType(true, (wchar_t*)type.c_str(), (int)type.length());

      if (type == L"TD") {
        // rename to th
        td->SetType(L"TH");
      }
    }
  }
}

void FigureTagSetAttributes(PdsStructElement* figure) {
  // TODO: add some other attributes
  figure->SetAlt(L"This is new image alternate text");
}

void PageContentsDidChange(void* data) {
  PsEvent* event = GetPdfix()->GetEvent();  
  std::cout << "PageContentsDidChange " << event->GetPage()->GetNumber() << std::endl;
}

void TagsEditStructTree(
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
  
  pdfix->RegisterEvent(kEventPageContentsDidChange, PageContentsDidChange, nullptr);
  
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
  
  // rename tags
  auto table_elem = FindStructElement(struct_tree, L"Table");
  if (table_elem)
    TableTagRowHeader(table_elem.get());

  // set tag attributes
  auto image_elem = FindStructElement(struct_tree, L"Figure");
  if (image_elem)
    FigureTagSetAttributes(image_elem.get());
  
  // change reading order
  
  // make top and bottom element on page an artifact (header, footer)
  auto page2 = FindStructElement(struct_tree, L"NonStruct", L"Page 2");

  // re-tag only one page
  auto page3 = FindStructElement(struct_tree, L"NonStruct", L"Page 3");
  
  // create struct element
  // add annot, add page object

  // reconstruct parent tree
  if (!struct_tree->UpdateParentTree())
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));

  // save document
  if (!doc->Save(save_path.c_str(), kSaveFull))
    throw std::runtime_error(std::to_string(GetPdfix()->GetErrorType()));
  
  pdfix->UnregisterEvent(kEventPageContentsDidChange, PageContentsDidChange, nullptr);
  
  doc->Close();
  pdfix->Destroy();
}
//! [TagsEditStructTree_cpp]
