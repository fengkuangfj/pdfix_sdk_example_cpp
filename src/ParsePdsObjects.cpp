////////////////////////////////////////////////////////////////////////////////////////////////////
// ParsePdsObjects.cpp
// Copyright (c) 2018 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
\page CPP_Samples C++ Samples
- \subpage ParsePdsObjects_cpp
*/
/*!
\page ParsePdsObjects_cpp Parse PDF document objects
Example how to read all documents objects in the document catalog.
\snippet /ParsePdsObjects.hpp ParsePdsObjects_cpp
*/

#include "pdfixsdksamples/ParsePdsObjects.h"

//! [ParsePdsObjects_cpp]
#include <string>
#include <map>
#include <iostream>
#include "pdfixsdksamples/Utils.h"
#include "Pdfix.h"

using namespace PDFixSDK;

#ifdef GetObject
#undef GetObject
#endif

namespace ParsePdsObjects {

  // ProcessObject gets the value of the object.
  void ProcessObject(PdsObject* obj, std::ostream& ss, std::string indent,
    std::map<PdsObject*, int>& mapped) {
    if (!obj) throw PdfixException();
    indent += "  ";

    auto found = mapped.find(obj);
    if (found != mapped.end()) {
      found->second++;
      ss << "R " << obj->GetId() << " 0 obj";
      return;
    }
    
    ss << "(" << obj->GetId() << ") : ";
    
    mapped.insert(std::make_pair(obj, 1));
    switch (obj->GetObjectType()) {
    case kPdsBoolean:{
      PdsBoolean* boolObj = (PdsBoolean*)obj;
      ss << (boolObj->GetValue() ? "true" : "false");
    } break;
    case kPdsNumber: {
      PdsNumber* numberObj = (PdsNumber*)obj;
      if (numberObj->IsIntegerValue())
        ss << numberObj->GetIntegerValue();
      else
        ss << (float)numberObj->GetValue();
    } break;
    case kPdsName: {
      PdsName* nameObj = (PdsName*)obj;
      auto str = nameObj->GetText();
      ss << ToUtf8(str);
    } break;
    case kPdsString: {
      PdsString* strObj = (PdsString*)obj;
      auto str = strObj->GetText();
      ss << ToUtf8(str);
    } break;
    case kPdsStream: {
      PdsStream* streamObj = (PdsStream*)obj;
      ss << "stream (" << streamObj->GetRawDataSize() << ")" << std::endl;
      auto sz = streamObj->GetSize();
      std::vector<unsigned char> data;
      if (sz) {
        data.resize(sz);
        if (!streamObj->Read(0, &data[0], sz))
          throw PdfixException();
        ss << std::string(begin(data), begin(data) + std::min(sz, 100)) <<
          "..." << std::endl << indent << "endstream ";
      }
      ProcessObject(streamObj->GetStreamDict(), ss, indent, mapped);
    } break;
    case kPdsArray: {
      PdsArray* arrObj = (PdsArray*)obj;
      for (int i = 0; i < (arrObj->GetNumObjects()); i++) {
        ss << std::endl << indent << "[" << i << "] ";
        ProcessObject(arrObj->Get(i), ss, indent, mapped);
      }
    } break;
    case kPdsDictionary: {
      PdsDictionary* dictObj = (PdsDictionary*)obj;
      for (int i = 0; i < (dictObj->GetNumKeys()); i++) {
        auto key = dictObj->GetKey(i);
        ss << indent << std::endl << indent << "/" << ToUtf8(key) << " ";
        std::string str;
        str.resize(dictObj->GetString(key.c_str(), nullptr, 0));
        dictObj->GetString(key.c_str(), (char*)str.c_str(), (int)str.size());
        ProcessObject(dictObj->Get(key.c_str()), ss, indent, mapped);
      }
    } break;
    case kPdsNull: {
      ss << "null";
    } break;
    default:
      ss << "Unknown Pdf Object";
    }
  }

  // Iterates all documents bookmars.
  void Run(
    const std::wstring& open_path,           // source PDF document
    std::ostream& output                     // output document
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
    auto obj = doc->GetObjectById(2120);
      
    PdsObject* root = doc->GetRootObject();
    if (!root)
      throw PdfixException();

    std::map<PdsObject*, int> mapped;
    output << "/root ";
    ProcessObject(root, output, "", mapped);

    doc->Close();
    pdfix->Destroy();
  }
}
//! [ParsePdsObjects_cpp]