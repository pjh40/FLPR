/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file flpr_interface.cc

  Extract interface definitions for top-level subroutines and functions
*/

#include "flpr/astruct.hh"
#include "flpr/flpr.hh"
#include <iostream>
#include <sstream>

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
  using Line_Buf = FLPR::Logical_File::Line_Buf;

public:
  void append_line(char const *const text) {
    output_buf_.emplace_back(std::string{text});
  }
  void append_line(std::string const &text) { output_buf_.push_back(text); }
  void append_line(std::string &&text) { output_buf_.emplace_back(text); }
  void append_line(FLPR::LL_Stmt const &stmt) {
    std::ostringstream str;
    FLPR::render(str, stmt.cbegin(), stmt.cend());
    output_buf_.emplace_back(str.str());
  }

  constexpr Line_Buf const &buf() const { return output_buf_; }

  bool operator()(File &file, Cursor c, bool const internal_procedure,
                  bool const module_procedure);

private:
  Line_Buf output_buf_;
};

bool do_file(std::string const &filename, FLPR::File_Type file_type,
             Interface_Action &action);

} // namespace FLPR_Interface

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: flpr_interface <fortran file>" << std::endl;
    return 1;
  }

  char const module_name[] = "foo";
  const std::string modstr{std::string{"module "} + std::string{module_name}};

  std::string const filename{argv[1]};
  FLPR_Interface::Interface_Action interface;
  interface.append_line(modstr);
  interface.append_line("interface");

  do_file(filename, FLPR::file_type_from_extension(filename), interface);

  interface.append_line("end interface");
  interface.append_line("end" + modstr);

  FLPR::Parsed_File output_file(interface.buf(), "output_name.f90",
                                FLPR::File_Type::FREEFMT);
  FLPR::Indent_Table indents;
  indents.apply_emacs_indent();
  output_file.indent(indents);
  for (auto const &ll : output_file.logical_lines()) {
    std::cout << ll;
  }

  return 0;
}

namespace FLPR_Interface {

/*--------------------------------------------------------------------------*/

bool do_file(std::string const &filename, FLPR::File_Type file_type,
             Interface_Action &visit_action) {

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
                                  bool const module_procedure) {
  if (internal_procedure || module_procedure) {
    return false;
  }

  Procedure proc(file);
  if (!proc.ingest(c)) {
    std::cerr << "\n******** Unable to ingest procedure *******\n" << std::endl;
    return false;
  }

  if (proc.is_main_program()) {
    return false;
  }

  auto proc_stmt = proc.cbegin(Procedure::PROC_BEGIN);
  assert(std::next(proc_stmt) == proc.cend(Procedure::PROC_BEGIN));
  auto stmt_cursor{proc_stmt->stmt_tree().ccursor()};
  int stmt_syntag = stmt_cursor->syntag;

  std::vector<std::string> dummy_names;
  switch (stmt_syntag) {
  case TAG(SG_SUBROUTINE_STMT): {
  } break;
  case TAG(SG_FUNCTION_STMT): {
    std::optional<FLPR::AST::Function_Stmt> ast =
        FLPR::AST::Function_Stmt::ingest(stmt_cursor);
    assert(ast.has_value());
    for (auto c : ast->dummy_arg_name_list) {
      dummy_names.push_back(c->token_range.front().text());
    }
    if (ast->suffix.has_value()) {
      if (ast->suffix->result_name) {
        dummy_names.push_back(
            ast->suffix->result_name->token_range.front().text());
      }
    }
  } break;
  default:
    std::cerr << "Got unexpected procedure statement of "
              << FLPR::Syntax_Tags::label(stmt_syntag) << std::endl;
  }
  for (auto const &n : dummy_names) {
    std::cout << ' ' << n;
  }
  std::cout << '\n';

  append_line(*proc_stmt);
  append_line(*(proc.cbegin(Procedure::PROC_END)));

  return true;
}
} // namespace FLPR_Interface
