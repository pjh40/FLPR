#include "astruct.hh"
#include <iostream>

#define INGEST_DEF(term) \
  std::optional<term> term::ingest(st_ref root)

#define ROOT_TAG_IS(X) \
  (FLPR::Syntax_Tags::X == root->syntag)

#define EXPECT_NAME \
  assert(FLPR::Syntax_Tags::is_name(root->syntag))

#define EXPECT_TAG(X) \
  if (root->syntag != (Syntax_Tags::X)) {                                     \
    std::cerr << "Expected root->syntag == "                                   \
              << Syntax_Tags::label(Syntax_Tags::X) << ", but got "        \
              << Syntax_Tags::label(root->syntag) << std::endl;            \
    assert (root->syntag == (Syntax_Tags::X)); \
  }


namespace FLPR {
namespace AST {

INGEST_DEF(Prefix) {
  return std::optional<Prefix>{};
}

INGEST_DEF(Function_Stmt) {
  if(ROOT_TAG_IS(SG_FUNCTION_STMT) && root.has_down()) {
    Function_Stmt ast;
    root.down();
    ast.prefix = Prefix::ingest(root);
    if(ROOT_TAG_IS(SG_PREFIX)) {
      root.next();
    }
    EXPECT_TAG(KW_FUNCTION);
    root.next();
    EXPECT_TAG(TK_NAME);
    ast.name = root;
    if(root.has_next()) {
      root.next();
      if(ROOT_TAG_IS(TK_PARENL)) root.next();
      if(ROOT_TAG_IS(SG_DUMMY_ARG_NAME_LIST)) {
        root.down();
        do {
          EXPECT_NAME;
          ast.dummy_arg_name_list.push_back(root);
        } while(root.try_next(2));
        root.up();
        root.next();
        EXPECT_TAG(TK_PARENR);
      }
    }
    return std::optional<Function_Stmt>{ast};
  }
  return std::optional<Function_Stmt>{};
}
  

}
}
