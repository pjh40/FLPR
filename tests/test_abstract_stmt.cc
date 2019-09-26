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

#define TAG(X) Syntax_Tags::X

bool function_stmt() {
  {
    AS_STMT(function_stmt, "function foo(a,b)");
    TEST_INT(ast.size(), 7);
    auto c = ast.ccursor();
    TEST_INT(c.num_branches(), 4);
    c.down();

    TEST_TOK(SG_PREFIX, c->syntag());
    TEST_FALSE(c->has_stmt_cursor());
    TEST_INT(c.num_branches(), 0);
    c.next();

    TEST_TOK(TK_NAME, c->syntag());
    TEST_TRUE(c->has_stmt_cursor());
    TEST_TOK(TK_NAME, c->stmt_cursor()->syntag);
    TEST_INT(c->stmt_cursor()->token_range.size(), 1);
    TEST_STR("foo", c->front_tt().text());
    TEST_INT(c.num_branches(), 0);
    c.next();

    TEST_TOK(SG_DUMMY_ARG_NAME_LIST, c->syntag());
    TEST_INT(c.num_branches(), 2);
    c.down();
    TEST_STR("a", c->front_tt().text());
    c.next();
    TEST_STR("b", c->front_tt().text());
    c.up();
    
    c.next();
    TEST_TOK(SG_SUFFIX, c->syntag());
  }
  return true;
}


int main() {
  TEST_MAIN_DECL;
  TEST(function_stmt);
  TEST_MAIN_REPORT;
}
