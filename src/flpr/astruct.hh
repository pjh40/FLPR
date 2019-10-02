#ifndef ASTRUCT_HH
#define ASTRUCT_HH

#include "Stmt_Tree.hh"
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

#define TAG(X) FLPR::Syntax_Tags::X

namespace FLPR {
namespace AST {

using st_ref = FLPR::Stmt::Stmt_Tree::const_cursor_t;


#define INGEST(term) \
  static std::optional<term> ingest(st_ref root)

struct Intrinsic_Type_Spec {
  /* The node of the type keyword (e.g. FLOAT) */
  st_ref type;
  /* The root of the kind specification */
  st_ref kind;
  INGEST(Intrinsic_Type_Spec);
};

struct Type_Class_Spec {
  /* The TYPE or CLASS node */
  st_ref type_or_class;  
  /* The root of the intrinsic-type-spec or derived-type-spec */
  st_ref spec;
  INGEST(Type_Class_Spec);
};

struct Declaration_Type_Spec {
  std::variant<Intrinsic_Type_Spec, Type_Class_Spec> v;
  INGEST(Declaration_Type_Spec);
};

struct Language_Binding_Spec {
  /* if set, the root of the scalar-default-char-constant-expr for NAME */
  st_ref name;
  INGEST(Language_Binding_Spec);
};

struct Prefix {
  /* All prefix-specs, apart from any declaration-type-spec */
  std::vector<st_ref> prefix_spec_list; 
  std::optional<Declaration_Type_Spec> declaration_type_spec;
  INGEST(Prefix);
};

struct Suffix {
  /* If this is set, it implies that there was a "RESULT ( result-name )"
     clause, and it is pointing to the result-name node */
  st_ref result_name;
  /* Because the NAME option to BIND is optional, we need to distinguish when
     proc-language-binding-spec exists (proc_language_binding_spec.has_value()),
     and when that spec has a name
     proc_language_binding_spec->name.associated()) */
  std::optional<Language_Binding_Spec> proc_language_binding_spec; 
};

struct Function_Stmt {
  std::optional<Prefix> prefix;
  st_ref name;
  std::vector<st_ref> dummy_arg_name_list;
  std::optional<Suffix> suffix;
  INGEST(Function_Stmt);
};

}
}
#undef TAG

#endif
