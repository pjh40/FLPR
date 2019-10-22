/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file Symbol_Table.cc

  Connect Entity_Info objects together in hierarchies with name-lookup
  capabilities.
*/


#include "Symbol_Table.hh"

namespace FLPR {

Entity_Info &Symbol_Table::operator[](key_type const &name) {
  auto ent = name_lookup_.find(name);
  if (ent == name_lookup_.end()) {
    symbols_.emplace_back(Sym_Tab_Entry{});
    Sym_Tab_Link link = std::prev(symbols_.end());
    link->value.name(name);
    name_lookup_.insert({name, link});
    return link->value;
  }
  return ent->second->value;
};

}
