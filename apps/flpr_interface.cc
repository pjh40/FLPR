/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*
  \file flpr_interface.cc

  Extract interface definitions for top-level subroutines and functions
*/

#include "flpr/flpr.hh"
#include "flpr/astruct.hh"
#include <iostream>

#define TAG(X) FLPR::Syntax_Tags::X

namespace FLPR_Interface {

using File = FLPR::Parsed_File<>;
using Cursor = typename File::Parse_Tree::cursor_t;
using Stmt_Cursor = typename File::Stmt_Cursor;
using Stmt_Range = typename File::Parse_Tree::value::Stmt_Range;
using Stmt_Iter = typename Stmt_Range::iterator;
using Stmt_Const_Iter = typename Stmt_Range::const_iterator;
using Procedure = FLPR::Procedure<File>;

class Interface_Action {
public:
  bool operator()(File &file, Cursor c, bool const internal_procedure,
                  bool const module_procedure) const;
};

bool do_file(std::string const &filename, FLPR::File_Type file_type,
             Interface_Action const &action);

} // namespace FLPR_Interface



int main(int argc, char *argv[]) {
  if(argc != 2) {
    std::cerr << "Usage: flpr_interface <fortran file>" << std::endl;
    return 1;
  }
  
  std::string const filename {argv[1]};
  FLPR_Interface::Interface_Action interface;
  do_file(filename, FLPR::file_type_from_extension(filename), interface);
  

  return 0;
}


namespace FLPR_Interface {

/*--------------------------------------------------------------------------*/

bool do_file(std::string const &filename, FLPR::File_Type file_type,
             Interface_Action const &visit_action) {

  File file(filename, file_type);
  if (!file)
    exit(1);

  FLPR::Procedure_Visitor puv(file, visit_action);
  bool const changed = puv.visit();
  return changed;
}

/*--------------------------------------------------------------------------*/


bool Interface_Action::operator()(File &file, Cursor c,
                                  bool const internal_procedure,
                                  bool const module_procedure) const {
  if (internal_procedure || module_procedure) {
    return false;
  }

  Procedure proc(file);
  if (!proc.ingest(c)) {
    std::cerr << "\n******** Unable to ingest procedure *******\n" << std::endl;
    return false;
  }

  if(proc.is_main_program()) {
    return false;
  }

  auto proc_stmt = proc.cbegin(Procedure::PROC_BEGIN);
  assert(std::next(proc_stmt) == proc.cend(Procedure::PROC_BEGIN));
  auto stmt_cursor {proc_stmt->stmt_tree().ccursor()};
  int stmt_syntag = stmt_cursor->syntag;
  
  std::vector<std::string> dummy_names;  
  switch(stmt_syntag) {
  case TAG(SG_SUBROUTINE_STMT):
    {
    }
    break;
  case TAG(SG_FUNCTION_STMT):
    {
      std::optional<FLPR::AST::Function_Stmt> ast =
        FLPR::AST::Function_Stmt::ingest(stmt_cursor);
      assert(ast.has_value());
      std::cout << "FUNCTION " << ast->name->token_range.front() << ": ";
      for(auto c : ast->dummy_arg_name_list) {
        std::cout << c->token_range.front() << ' ';
      }
      std::cout << '\n';
    }
    break;
  default:
    std::cerr << "Got unexpected procedure statement of "
              << FLPR::Syntax_Tags::label(stmt_syntag) << std::endl;
  }
  for(auto const &n : dummy_names) {
    std::cout << ' ' << n;
  }
  std::cout << '\n';
  
  return true;
}
}
