/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*
  Tests for full statement parsers.
*/

#include "flpr/abstract_stmt.hh"
#include "parse_helpers.hh"

using namespace FLPR;

#define AS_STMT(SP, S)                                                         \
    LL_Helper l({S});                                                          \
    TT_Stream ts = l.stream1();                                                \
    FLPR::Stmt::Stmt_Tree st = FLPR::Stmt::SP(ts);                      \
    FLPR::AST::AS_Tree ast = FLPR::AST::SP(st.cursor())                          


bool function_stmt() {
  AS_STMT(function_stmt, "function foo(a)");
  return true;
}


int main() {
  TEST_MAIN_DECL;
  TEST(function_stmt);
  TEST_MAIN_REPORT;
}
