#ifndef ASTRUCT_HH
#define ASTRUCT_HH

#include "Stmt_Tree.hh"
#include <optional>
#include <tuple>
#include <vector>

#define TAG(X) FLPR::Syntax_Tags::X

namespace AST {

using st_ref = FLPR::Stmt::Stmt_Tree::const_cursor_t;

struct Declaration_Type_Spec {
  
};

struct Prefix_Spec {

};

struct Prefix {
  std::vector<Prefix_Spec> prefix_spec_list;
};



}

#undef TAG

#endif
