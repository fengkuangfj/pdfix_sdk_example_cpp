////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtractPageMap.cpp
// Copyright (c) 2020 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pdfixsdksamples/ExtractData.h"

namespace ExtractData {
  // extract text element
  void ExtractTextElement(PdeText* text, ptree& node, const DataType& data_types) {
    node.put("text", EncodeText(text->GetText()));

    if (data_types.text_state) {
      ptree text_state_node;
      PdfTextState ts;
      text->GetTextState(&ts);
      ExtractTextState(&ts, text_state_node, data_types);
      node.put_child("text_state", text_state_node);
    }
  }

  // extract table element
  void ExtractTableElement(PdeTable* table, ptree& node, const DataType& data_types) {
    node.put("num_colls", table->GetNumCols());
    node.put("num_rows", table->GetNumRows());

    ptree rows_node;
    for (int row = 0; row < table->GetNumRows(); row++) {
      ptree cols_node;
      for (int col = 0; col < table->GetNumCols(); col++) {
        auto cell = table->GetCell(row, col);
        if (!cell)
          throw PdfixException();
        ptree cell_node;
        ExtractPageElement(cell, cell_node, data_types);
        cols_node.push_back(std::make_pair("", cell_node));
      }
      rows_node.push_back(std::make_pair("", cols_node));
    }
    node.put_child("rows", rows_node);
  }

  // extract image element
  void ExtractImageElement(PdeImage* image, ptree& node, const DataType& data_types) {
    auto page = image->GetPageMap()->GetPage();
    // render this element only
    image->SetRender(true);
    auto bbox = image->GetBBox();
    RenderPageArea(page, bbox, node, data_types);
    // render cleanup
    image->SetRender(false);
  }

  // write page element
  void ExtractPageElement(PdeElement* element, ptree& node, const DataType& data_types) {
    auto get_element_type_string = [&]() {
      std::string type = "unknown";
      switch (element->GetType()) {
      case kPdeText: return std::string("pde_text");
      case kPdeTextLine: return std::string("pde_text_line");
      case kPdeWord: return std::string("pde_word");
      case kPdeTextRun: return std::string("pde_text_run");
      case kPdeImage: return std::string("pde_image");
      case kPdeContainer: return std::string("pde_container");
      case kPdeList: return std::string("pde_list");
      case kPdeLine: return std::string("pde_line");
      case kPdeRect: return std::string("pde_rect");
      case kPdeTable: return std::string("pde_table");
      case kPdeCell: return std::string("pde_cell");
      case kPdeToc: return std::string("pde_toc");
      case kPdeFormField: return std::string("pde_form_field");
      case kPdeHeader: return std::string("pde_header");
      case kPdeFooter: return std::string("pde_footer");
      case kPdeAnnot: return std::string("pde_annot");
      default: return std::string("unknown");
      }
    };
    node.put("type", get_element_type_string());

    if (data_types.extract_bbox) {
      ptree bbox_node;
      ExtractBBox(element->GetBBox(), bbox_node, data_types);
      node.put_child("bbox", bbox_node);
    }

    switch (element->GetType()) {
      case kPdeText: 
        if (data_types.extract_text) 
          ExtractTextElement((PdeText *)element, node, data_types);
        break;
      case kPdeTable:
        if (data_types.extract_tables)
          ExtractTableElement((PdeTable *)element, node, data_types);
        break;
      case kPdeImage:
        if (data_types.extract_images)
          ExtractImageElement((PdeImage *)element, node, data_types);
        break;
      default:;
      }

    // kids
    ptree kids_node;
    for (int i = 0; i < element->GetNumChildren(); i++) {
      ptree kid_node;
      ExtractPageElement(element->GetChild(i), kid_node, data_types);
      kids_node.push_back(std::make_pair("", kid_node));
    }
    if (kids_node.size())
      node.put_child("kids", kids_node);
  }

  // process page map
  void ExtractPageMap(PdePageMap* page_map, ptree& node, const DataType& data_types) {
    auto element = page_map->GetElement();
    if (!element)
      throw PdfixException();

    ptree element_node;
    ExtractPageElement(element, element_node, data_types);
    node.put_child("elements", element_node);
  }
}