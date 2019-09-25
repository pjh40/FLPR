/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file abstract_stmt.cc
*/

#include "flpr/abstract_stmt.hh"
#include <iostream>
namespace FLPR {
namespace AST {

#define TAG(X) Syntax_Tags::X

bool back_subtree_filled(AS_Tree const & tree) {
  return tree.croot()->branches().back().size() > 1;
}

#define EXPECT(TEVAL, TACTUAL) \
  if((TACTUAL) != (Syntax_Tags::TEVAL)) {                               \
    std::cerr << "Expected " #TACTUAL "=" << Syntax_Tags::label(Syntax_Tags::TEVAL) << ", but got " << Syntax_Tags::label(TACTUAL) << std::endl; \
  assert((TACTUAL) == (Syntax_Tags::TEVAL)); }


AS_Tree dummy_arg_name_list(st_cursor stc) {
  AS_Tree ast{TAG(SG_DUMMY_ARG_NAME_LIST)};
  if(TAG(SG_DUMMY_ARG_NAME_LIST) == stc->syntag) { 
    stc.down();
    do {
      assert(Syntax_Tags::is_name(stc->syntag));
      ast.graft_back(AS_Tree{TAG(TK_NAME), stc});
      if(stc.has_next()) {
        stc.next();
        EXPECT(TK_COMMA, stc->syntag);
      }
    } while(stc.try_next());
  }
  return ast;
}

AS_Tree function_stmt(st_cursor stc) {
  // FUNCTION < prefix name args suffix>
  AS_Tree ast{TAG(KW_FUNCTION)};
  if(TAG(SG_FUNCTION_STMT) == stc->syntag) {
    stc.down();
    ast.graft_back(prefix(stc)); // prefix is always there
    stc.next();
    EXPECT(KW_FUNCTION, stc->syntag);
    stc.next();
    EXPECT(TK_NAME, stc->syntag);
    ast.graft_back(AS_Tree{TAG(TK_NAME), stc});
    stc.next();
    EXPECT(TK_PARENL, stc->syntag);
    stc.next();
    ast.graft_back(dummy_arg_name_list(stc));
    if(back_subtree_filled(ast)) stc.next();
    EXPECT(TK_PARENR, stc->syntag);
    stc.try_next();
    ast.graft_back(suffix(stc));
  }
  
  return ast;
}

AS_Tree prefix(st_cursor stc) {
  AS_Tree ast{TAG(SG_PREFIX)};
  return ast;
}

AS_Tree suffix(st_cursor stc) {
  AS_Tree ast{TAG(SG_SUFFIX)};
  return ast;
}

} // namespace AST
} // namespace FLPR
