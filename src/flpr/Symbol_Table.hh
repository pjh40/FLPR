/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file Symbol_Table.hh

  Connect Entity_Info objects together in hierarchies with name-lookup
  capabilities.
*/

#include "flpr/Entity_Info.hh"
#include "flpr/Safe_List.hh"
#include <unordered_map>

namespace FLPR {




class Symbol_Table {
public:
  using key_type = std::string;
  using mapped_type = Entity_Info;

public:
  Entity_Info &operator[](std::string const &name);
  
private:
  struct Sym_Tab_Entry {
    mapped_type value;
    using link = Safe_List<Sym_Tab_Entry>::iterator;
    std::vector<link> preds;
  };

  using Sym_Tab_Rep = Safe_List<Sym_Tab_Entry>;
  using Sym_Tab_Link = typename Sym_Tab_Rep::iterator;
  using Lookup_Rep = std::unordered_map<key_type, Sym_Tab_Link>; 
  
  Sym_Tab_Rep symbols_;
  Lookup_Rep name_lookup_;
  
};
}
