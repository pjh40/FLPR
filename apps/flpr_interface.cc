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
#include "flpr/Symbol_Table.hh"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

#define TAG(X) FLPR::Syntax_Tags::X

namespace FLPR_Interface {

using File = FLPR::Parsed_File<>;
using Cursor = typename File::Parse_Tree::cursor_t;
using Stmt_Cursor = typename File::Stmt_Cursor;
using Stmt_Range = typename File::Parse_Tree::value::Stmt_Range;
using Stmt_Iter = typename Stmt_Range::iterator;
using Stmt_Const_Iter = typename Stmt_Range::const_iterator;
using Procedure = FLPR::Procedure<File>;
using Prgm_Const_Cursor = typename Procedure::Prgm_Const_Cursor;

class Interface_Action {
public:
  using Line_Buf = FLPR::Logical_File::Line_Buf;
  using Dummies = std::unordered_set<std::string>;

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
  void append_blank_line() { output_buf_.emplace_back(std::string{}); }

  constexpr Line_Buf const &buf() const { return output_buf_; }

  bool operator()(File &file, Cursor c, bool const internal_procedure,
                  bool const module_procedure);

private:
  bool process_spec_(Prgm_Const_Cursor pc, Dummies const &dummy_args);

  Line_Buf output_buf_;
};

bool do_file(std::string const &filename, FLPR::File_Type file_type,
             Interface_Action &action);

} // namespace FLPR_Interface

int main(int argc, char *argv[]) {

  FLPR::Symbol_Table symtab;
  
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

  Dummies dummy_names;
  switch (stmt_syntag) {
  case TAG(SG_SUBROUTINE_STMT): {
    auto ast = FLPR::AST::Subroutine_Stmt::ingest(stmt_cursor);
    assert(ast.has_value());
    for (auto c : ast->dummy_arg_list) {
      assert(TAG(SG_DUMMY_ARG) == c->syntag);
      c.down();
      if (FLPR::Syntax_Tags::is_name(c->syntag)) {
        dummy_names.insert(c->token_range.front().text());
      }
    }
  } break;
  case TAG(SG_FUNCTION_STMT): {
    std::optional<FLPR::AST::Function_Stmt> ast =
        FLPR::AST::Function_Stmt::ingest(stmt_cursor);
    assert(ast.has_value());
    for (auto c : ast->dummy_arg_name_list) {
      dummy_names.insert(c->token_range.front().text());
    }
    if (ast->suffix.has_value() && ast->suffix->result_name) {
      dummy_names.insert(ast->suffix->result_name->token_range.front().text());
    } else {
      dummy_names.insert(ast->name->token_range.front().text());
    }
  } break;
  default:
    std::cerr << "Got unexpected procedure statement of "
              << FLPR::Syntax_Tags::label(stmt_syntag) << std::endl;
  }

  append_line(*proc_stmt);
  for (auto u = proc.cbegin(Procedure::USES); u != proc.cend(Procedure::USES);
       u++) {
    append_line(*u);
  }
  for (auto i = proc.cbegin(Procedure::IMPLICITS);
       i != proc.cend(Procedure::IMPLICITS); i++) {
    append_line(*i);
  }

  if (proc.has_region(Procedure::DECLS)) {
    bool changed = false;
    auto pc = proc.range_cursor(Procedure::DECLS);
    if (pc) {
      pc.down();
      do {
        if (TAG(PG_DECLARATION_CONSTRUCT) != pc->syntag())
          continue;
        pc.down();
        if (TAG(PG_SPECIFICATION_CONSTRUCT) == pc->syntag()) {
          pc.down();
          changed |= process_spec_(pc, dummy_names);
          pc.up();
        }
        pc.up();
      } while (pc.try_next());
    }
  }

  append_line(*(proc.cbegin(Procedure::PROC_END)));
  append_blank_line();

  return true;
}

bool has_parameter_attrib(FLPR::AST::Type_Decl_Attr_Seq const &seq) {
  for (auto const &as : seq.attr_spec_list) {
    if (TAG(KW_PARAMETER) == as.attr_spec->syntag)
      return true;
  }
  return false;
}

bool Interface_Action::process_spec_(Prgm_Const_Cursor prgm_cursor,
                                     Dummies const &dummy_args) {

  switch (prgm_cursor->syntag()) {
  case TAG(PG_DERIVED_TYPE_DEF): {
    /* Copy derived type definitions out in case they appear in dummy parameter
       declarations */
    for (auto const &st : prgm_cursor->stmt_range()) {
      std::ostringstream output_line;
      st.render(output_line);
      append_line(output_line.str());
    }
  } break;
  case TAG(SG_PARAMETER_STMT): {
    assert(prgm_cursor->is_stmt());
    std::ostringstream output_line;
    prgm_cursor->stmt_range().front().render(output_line);
    append_line(output_line.str());
  } break;
  case TAG(SG_TYPE_DECLARATION_STMT): {
    assert(prgm_cursor->is_stmt());
    auto stmt_cursor = prgm_cursor->stmt_tree().ccursor();
    auto ast = FLPR::AST::Type_Declaration_Stmt::ingest(stmt_cursor);
    assert(ast.has_value());

    std::ostringstream output_line;

    if (has_parameter_attrib(ast->type_decl_attr_seq)) {
      prgm_cursor->stmt_range().front().render(output_line);
      append_line(output_line.str());
      return true;
    }

    int dummy_found = 0;

    for (auto const &ed : ast->entity_decl_list) {
      auto const &var_name = ed.name->token_range.front().text();
      if (dummy_args.count(var_name)) {
        if (!dummy_found) {
          ast->type_decl_attr_seq.self->token_range.render(output_line);
          output_line << ' ';
        } else {
          output_line << ", ";
        }
        auto c = ed.name;
        c.up();
        c->token_range.render(output_line);
        dummy_found += 1;
      }
    }
    if (dummy_found) {
      append_line(output_line.str());
    }
  } break;
  case TAG(SG_OTHER_SPECIFICATION_STMT): {
    assert(prgm_cursor->is_stmt());
    auto stmt_cursor = prgm_cursor->stmt_tree().ccursor();
  }   break;
  default:
    std::cerr << "Skipping " << FLPR::Syntax_Tags::label(prgm_cursor->syntag())
              << " in specification-part\n";
  }

  return false;
}

} // namespace FLPR_Interface
