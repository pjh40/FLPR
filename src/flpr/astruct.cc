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

#define EXPECT_OPTIONAL(LHS, RHS) \
  { auto tmp_optional = RHS; assert(tmp_optional.has_value()); LHS = std::move(*tmp_optional); }

namespace FLPR {
namespace AST {

INGEST_DEF(Attr_Spec) {
  if (ROOT_TAG_IS(SG_ATTR_SPEC) && root.has_down()) {
    Attr_Spec ast;
    ast.attr_spec = root;  /* FIXME: true, but lazy. */
    return std::optional<Attr_Spec>{ast};
  }
  return std::optional<Attr_Spec>{};
}


INGEST_DEF(Declaration_Type_Spec) {
  if(ROOT_TAG_IS(SG_DECLARATION_TYPE_SPEC) && root.has_down()) {
    Declaration_Type_Spec ast;
    root.down();
    if(ROOT_TAG_IS(SG_INTRINSIC_TYPE_SPEC)) {
      auto its = Intrinsic_Type_Spec::ingest(root);
      assert(its.has_value());
      ast.v.emplace<0>(std::move(*its));
    } else {
      auto tcs = Type_Class_Spec::ingest(root);
      assert(tcs.has_value());
      ast.v.emplace<1>(std::move(*tcs));
    }
    return std::optional<Declaration_Type_Spec>{ast};
  }
  return std::optional<Declaration_Type_Spec>{};
}

INGEST_DEF(Entity_Decl) {
  if (ROOT_TAG_IS(SG_ENTITY_DECL) && root.has_down()) {
    Entity_Decl ast;
    root.down();
    EXPECT_NAME;
    ast.name = root;
    if(!root.has_next()) return std::optional<Entity_Decl>{ast};
    root.next();
    if(ROOT_TAG_IS(TK_PARENL)) {
      root.next();
      EXPECT_TAG(SG_ARRAY_SPEC);
      ast.array_spec = root;
      root.next();
      EXPECT_TAG(TK_PARENR);
      if(!root.has_next()) return std::optional<Entity_Decl>{ast};
    }
    if(ROOT_TAG_IS(TK_BRACKETL)) {
      root.next();
      EXPECT_TAG(SG_COARRAY_SPEC);
      ast.coarray_spec = root;
      root.next();
      EXPECT_TAG(TK_BRACKETR);
      if(!root.has_next()) return std::optional<Entity_Decl>{ast};
    }
    if(ROOT_TAG_IS(TK_ASTERISK)) {
      root.next();
      EXPECT_TAG(SG_CHAR_LENGTH);
      if(!root.has_next()) return std::optional<Entity_Decl>{ast};
    }
    EXPECT_TAG(SG_INITIALIZATION);
    ast.initialization = root;
    return std::optional<Entity_Decl>{ast};
  }
  return std::optional<Entity_Decl>{};
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
    EXPECT_NAME;
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
    if (ROOT_TAG_IS(SG_INTEGER_TYPE_SPEC)) {
      root.down();
    }
    bool expect_char_selector{false};
    if (ROOT_TAG_IS(KW_CHARACTER)) {
      expect_char_selector = true;
    }
    ast.type = root;
    if(root.has_next()) {
      root.next();
      if(expect_char_selector) {
        EXPECT_TAG(SG_CHAR_SELECTOR);
      } else {
        EXPECT_TAG(SG_KIND_SELECTOR);
      }
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

INGEST_DEF(Subroutine_Stmt) {
  if(ROOT_TAG_IS(SG_SUBROUTINE_STMT) && root.has_down()) {
    Subroutine_Stmt ast;
    root.down();
    ast.prefix = Prefix::ingest(root);
    if(ROOT_TAG_IS(SG_PREFIX)) {
      root.next();
    }
    EXPECT_TAG(KW_SUBROUTINE);
    root.next();
    EXPECT_TAG(TK_NAME);
    ast.name = root;
    if(root.has_next()) {
      root.next();
      if(ROOT_TAG_IS(TK_PARENL)) root.next();
      if(ROOT_TAG_IS(SG_DUMMY_ARG_LIST)) {
        root.down();
        do {
          ast.dummy_arg_list.push_back(root);
        } while(root.try_next(2));
        root.up();
        root.next();
      }
      EXPECT_TAG(TK_PARENR);
      if (root.has_next()) {
        root.next();
        EXPECT_TAG(SG_PROC_LANGUAGE_BINDING_SPEC);
        ast.proc_language_binding_spec = Language_Binding_Spec::ingest(root);
      }
    }
    return std::optional<Subroutine_Stmt>{ast};
  }
  return std::optional<Subroutine_Stmt>{};
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

INGEST_DEF(Type_Decl_Attr_Seq) {
  if (ROOT_TAG_IS(SG_TYPE_DECL_ATTR_SEQ) && root.has_down()) {
    Type_Decl_Attr_Seq ast;
    ast.self = root;
    root.down();
    EXPECT_OPTIONAL(ast.declaration_type_spec,
                    Declaration_Type_Spec::ingest(root));
    if(root.has_next()) {
      root.next();
      while(ROOT_TAG_IS(TK_COMMA) && root.has_next()) {
        root.next();
        auto tmp_optional = Attr_Spec::ingest(root);
        assert(tmp_optional.has_value());
        ast.attr_spec_list.emplace_back(std::move(*tmp_optional));
        root.next();
      }
    }
    return std::optional<Type_Decl_Attr_Seq>{ast};
  }
  return std::optional<Type_Decl_Attr_Seq>{};
}

INGEST_DEF(Type_Declaration_Stmt) {
  if (ROOT_TAG_IS(SG_TYPE_DECLARATION_STMT) && root.has_down()) {
    Type_Declaration_Stmt ast;
    root.down();
    EXPECT_OPTIONAL(ast.type_decl_attr_seq,
                    Type_Decl_Attr_Seq::ingest(root));
    root.next();
    EXPECT_TAG(SG_ENTITY_DECL_LIST);
    root.down();
    do {
      if(ROOT_TAG_IS(TK_COMMA)) root.next();
      EXPECT_TAG(SG_ENTITY_DECL);
      auto tmp_optional = Entity_Decl::ingest(root);
      assert(tmp_optional.has_value());
      ast.entity_decl_list.emplace_back(std::move(*tmp_optional));
    } while(root.try_next());
    return std::optional<Type_Declaration_Stmt>{ast};
  }
  return std::optional<Type_Declaration_Stmt>{};
}

}
}
