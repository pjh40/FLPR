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

INGEST_DEF(Declaration_Type_Spec) {
  if(ROOT_TAG_IS(SG_DECLARATION_TYPE_SPEC) && root.has_down()) {
    Declaration_Type_Spec ast;
    root.down();
    if(ROOT_TAG_IS(SG_INTRINSIC_TYPE_SPEC)) {
      auto its = Intrinsic_Type_Spec::ingest(root);
      ast.v.emplace<0>(std::move(its.value()));
    } else {
      auto tcs = Type_Class_Spec::ingest(root);
      ast.v.emplace<1>(std::move(tcs.value()));
    }
    return std::optional<Declaration_Type_Spec>{ast};
  }
  return std::optional<Declaration_Type_Spec>{};
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
      }
      EXPECT_TAG(TK_PARENR);
      if (root.has_next()) {
        root.next();
        EXPECT_TAG(SG_SUFFIX);
        ast.suffix = Suffix::ingest(root);
      }
    }
    return std::optional<Function_Stmt>{ast};
  }
  return std::optional<Function_Stmt>{};
}

INGEST_DEF(Intrinsic_Type_Spec) {
  if (ROOT_TAG_IS(SG_INTRINSIC_TYPE_SPEC) && root.has_down()) {
    Intrinsic_Type_Spec ast;
    root.down();
    ast.type = root;
    if(root.has_next()) {
      root.next();
      EXPECT_TAG(SG_KIND_SELECTOR);
      ast.kind = root;
    }
    return std::optional<Intrinsic_Type_Spec>{ast};
  }
  return std::optional<Intrinsic_Type_Spec>{};
}

INGEST_DEF(Language_Binding_Spec) {
  if (ROOT_TAG_IS(SG_LANGUAGE_BINDING_SPEC) && root.has_down()) {
    Language_Binding_Spec ast;
    root.down();
    EXPECT_TAG(KW_BIND);
    root.next();
    EXPECT_TAG(TK_PARENL);
    root.next();
    EXPECT_TAG(TK_NAME);
    root.next();
    if (ROOT_TAG_IS(TK_COMMA)) {
      root.next();
      EXPECT_TAG(KW_NAME);
      root.next();
      EXPECT_TAG(TK_EQUAL);
      root.next();
      ast.name = root;
    }
    return std::optional<Language_Binding_Spec>{ast};
  }
  return std::optional<Language_Binding_Spec>{};
}

INGEST_DEF(Prefix) {
  if(ROOT_TAG_IS(SG_PREFIX) && root.has_down()) {
    Prefix ast;
    root.down();
    do {
      EXPECT_TAG(SG_PREFIX_SPEC);
      root.down();
      if(ROOT_TAG_IS(SG_DECLARATION_TYPE_SPEC)) {
        ast.declaration_type_spec = Declaration_Type_Spec::ingest(root);
      } else {
        ast.prefix_spec_list.push_back(root);
      }
      root.up();
    } while(root.try_next());
    return std::optional<Prefix>{ast};
  }
  return std::optional<Prefix>{};
}

INGEST_DEF(Suffix) {
  if (ROOT_TAG_IS(SG_SUFFIX) && root.has_down()) {
    Suffix ast;
    root.down();
    do {
      if(ROOT_TAG_IS(KW_RESULT)) {
        root.next();
        EXPECT_TAG(TK_PARENL);
        root.next();
        EXPECT_NAME;
        ast.result_name = root;
        // Consume everything so that the try_next loop will be in a sane place
        root.next();
        EXPECT_TAG(TK_PARENR);
      }
      else if(ROOT_TAG_IS(SG_PROC_LANGUAGE_BINDING_SPEC)) {
        root.down();
        EXPECT_TAG(SG_LANGUAGE_BINDING_SPEC);
        ast.proc_language_binding_spec = Language_Binding_Spec::ingest(root);
      }
      else {
        std::cerr << "Unexpected tag: " << Syntax_Tags::label(root->syntag)
                  << std::endl;
        assert(0);
      }
    } while(root.try_next());
    return std::optional<Suffix>{ast};
  }
  return std::optional<Suffix>{};
}

INGEST_DEF(Type_Class_Spec) {
  if (ROOT_TAG_IS(KW_TYPE) || ROOT_TAG_IS(KW_CLASS)) {
    Type_Class_Spec ast;
    ast.type_or_class = root;
    root.next();
    EXPECT_TAG(TK_PARENL);
    root.next();
    ast.spec = root;
    return std::optional<Type_Class_Spec>{ast};
  }
  return std::optional<Type_Class_Spec>{};
}

}
}
