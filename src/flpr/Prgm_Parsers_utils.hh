/*
   Copyright (c) 2019-2020, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*
  TEMPLATE CLASS IMPLEMENTATION

  Note that this gets directly included into the Prgm_Parsers class definition!

  These are parser utilities.
*/

#if FLPR_PRGM_PARSERS_HH

//! Shorthand for defining a Prgm Parser rule
#define PPARSER(P)                                                             \
  template <typename Node_Data>                                                \
  auto Parsers<Node_Data>::P(State &state)->PP_Result

//! Shorthand for accessing a Syntax_Tag
#define TAG(X) Syntax_Tags::X

//! Shorthand for accessing a Stmt
#define STMT(X) stmt(FLPR::Stmt::X)

#if FLPR_TRACE_PG
#define RULE(T)                                                                \
  constexpr auto rule_tag{Syntax_Tags::T};                                     \
  Syntax_Tags::print(std::cerr << "PGTRACE >  ", Syntax_Tags::T) << '\n'

#define EVAL(T, E)                                                             \
  PP_Result res_ = E;                                                          \
  if (!res_.match)                                                             \
    Syntax_Tags::print(std::cerr << "PGTRACE <! ", Syntax_Tags::T) << '\n';    \
  else                                                                         \
    std::cerr << "PGTRACE <= " << res_.parse_tree << '\n';                     \
  return res_;
#else
#define RULE(T)                                                                \
  constexpr auto rule_tag { Syntax_Tags::T }
#define EVAL(T, E) return E
#endif

//! Append the \p donor subtree to the branches of \p t
/*! If the root of the donor subtree has the pseudo syntag of HOIST, each branch
    of the donor subtree gets hoisted up, meaning it gets added as an
    independent branch of t.  This is an emplace-like operation: donor is junk
    after this call. */
static void hoist_back(Prgm_Tree &t, Prgm_Tree &&donor) {
  if ((*donor)->syntag() == Syntax_Tags::HOIST) {
    for (auto &&b : donor->branches()) {
      auto new_loc = t->branches().emplace(t->branches().end(), std::move(b));
      new_loc->link(new_loc, t.root());
    }
  } else {
    t.graft_back(std::move(donor));
  }
}

//! Update a node's stmt_range to cover its branches. Non-recursive.
static void cover_branches(typename Prgm_Tree::reference node) {
  node->stmt_range().clear();
  if (node->is_stmt()) {
    node->stmt_range().push_back(node->ll_stmt_iter());
  } else {
    for (auto &b : node.branches()) {
      node->stmt_range().push_back(b->stmt_range());
    }
  }
}

/******************************* COMBINATORS **********************************/

//! Return a result if any of the alternatives match
/*! This "covers" the result of a matching parser with a new syntag node */
template <typename... Ps> class Alternatives_Parser {
public:
  Alternatives_Parser(Alternatives_Parser const &) = default;
  constexpr explicit Alternatives_Parser(int const syntag, Ps &&... ps) noexcept
      : syntag_{syntag}, parsers_{std::forward<Ps>(ps)...} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root;
    auto assign_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (match) {
        root = std::move(pt);
      }
      return match;
    };
    auto fold = [&assign_if](auto... as) constexpr {
      return FLPR::details_::or_fold(assign_if, as...);
    };
    bool const match = std::apply(fold, parsers_);
    if (match) {
      Prgm_Tree new_root{this->syntag_};
      hoist_back(new_root, std::move(root));
      if (new_root)
        cover_branches(*new_root);
      return PP_Result{std::move(new_root), true};
    }
    if (root)
      cover_branches(*root);
    return PP_Result{std::move(root), false};
  }

private:
  int const syntag_;
  std::tuple<Ps...> const parsers_;
};

//! Generate an Alternatives_Parser
template <typename... Ps>
static constexpr Alternatives_Parser<Ps...> alts(int const syntag,
                                                 Ps &&... ps) noexcept {
  return Alternatives_Parser<Ps...>{syntag, std::forward<Ps>(ps)...};
}

//! Returns true if the end of the statement stream has been reached
class End_Of_Stream_Parser {
public:
  PP_Result operator()(State &state) const noexcept {
    if (state.ss) {
      std::cerr << "Unrecognized statement\n";
      state.ss->print_me(std::cerr, false)
          << "\nwhen expecting end-of-stream " << std::endl;
      return PP_Result{Prgm_Tree{}, false};
    }
    return PP_Result{Prgm_Tree{}, true};
  }
};

//! Generate a End_Of_Stream_Parser
static constexpr auto end_stream() { return End_Of_Stream_Parser{}; }

//! Forwards the Prgm_Tree of parser_, always setting match to true
template <typename P> class Optional_Parser {
public:
  Optional_Parser(Optional_Parser const &) = default;
  constexpr explicit Optional_Parser(P &&p) noexcept
      : parser_{std::forward<P>(p)} {}
  PP_Result operator()(State &state) const noexcept {
    auto [pt, match] = parser_(state);
    if (pt) {
      cover_branches(*pt);
    }
    return PP_Result{std::move(pt), true};
  }

private:
  P const parser_;
};

//! Generate an Optional_Parser
template <typename P> static constexpr Optional_Parser<P> opt(P &&p) noexcept {
  return Optional_Parser<P>{std::forward<P>(p)};
}

//! Match if all specified parsers match
/*! On match, this returns a Stmt_Tree with a root \c syntag that covers all the
  Stmt_Trees from the sub parsers */
template <typename... Ps> class Opt_Sequence_Parser {
public:
  Opt_Sequence_Parser(Opt_Sequence_Parser const &) = default;
  constexpr explicit Opt_Sequence_Parser(int const syntag, Ps &&... ps) noexcept
      : syntag_{syntag}, parsers_{std::forward<Ps>(ps)...} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root(syntag_);
    auto attach_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (pt) {
        hoist_back(root, std::move(pt));
      }
      return match;
    };
    auto fold = [&attach_if](auto... as) constexpr {
      return FLPR::details_::and_fold(attach_if, as...);
    };
    std::apply(fold, parsers_);
    if (root)
      cover_branches(*root);
    return PP_Result{std::move(root), true};
  }

private:
  int const syntag_;
  std::tuple<Ps...> const parsers_;
};

//! Generate a Opt_Sequence_Parser
template <typename... Ps>
static constexpr Opt_Sequence_Parser<Ps...> opt_seq(int const syntag,
                                                    Ps &&... ps) noexcept {
  return Opt_Sequence_Parser<Ps...>{syntag, std::forward<Ps>(ps)...};
}

//! Generate a HOIST Opt_Sequence_Parser
template <typename... Ps>
static constexpr Opt_Sequence_Parser<Ps...> h_opt_seq(Ps &&... ps) noexcept {
  return Opt_Sequence_Parser<Ps...>{Syntax_Tags::HOIST,
                                    std::forward<Ps>(ps)...};
}

//! Kleene plus: One or more matches of the parser
template <typename P> class Plus_Parser {
public:
  Plus_Parser(Plus_Parser const &) = default;
  constexpr explicit Plus_Parser(int const syntag, P &&p) noexcept
      : syntag_{syntag}, parser_{std::forward<P>(p)} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root(syntag_);
    auto attach_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (pt) {
        hoist_back(root, std::move(pt));
      }
      return match;
    };
    if (!state.ss || !attach_if(parser_))
      return PP_Result{};
    while (state.ss && attach_if(parser_))
      ;
    if (root)
      cover_branches(*root);
    return PP_Result{std::move(root), true};
  }

private:
  int const syntag_;
  P const parser_;
};

//! Generate a Plus_Parser
template <typename P>
static constexpr Plus_Parser<P> plus(int const syntag, P &&p) noexcept {
  return Plus_Parser<P>{syntag, std::forward<P>(p)};
}

//! Match if all specified parsers match
/*! On match, this returns a Stmt_Tree with a root \c syntag that covers all the
 * Stmt_Trees from the sub parsers */
template <typename... Ps> class Sequence_Parser {
public:
  Sequence_Parser(Sequence_Parser const &) = default;
  constexpr explicit Sequence_Parser(int const syntag, Ps &&... ps) noexcept
      : syntag_{syntag}, parsers_{std::forward<Ps>(ps)...} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root(syntag_);
    auto attach_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (pt) {
        hoist_back(root, std::move(pt));
      }
      return match;
    };
    auto fold = [&attach_if](auto... as) constexpr {
      return FLPR::details_::and_fold(attach_if, as...);
    };
    bool const match = std::apply(fold, parsers_);
    if (!match) {
      std::cerr << "Unrecognized statement\n";
      state.ss->print_me(std::cerr, false) << "\nwhile parsing ";
      FLPR::Syntax_Tags::print(std::cerr, syntag_) << std::endl;
      return PP_Result{};
    }
    if (root)
      cover_branches(*root);
    return PP_Result{std::move(root), match};
  }

private:
  int const syntag_;
  std::tuple<Ps...> const parsers_;
};

//! Generate a Sequence_Parser
template <typename... Ps>
static constexpr Sequence_Parser<Ps...> seq(int const syntag,
                                            Ps &&... ps) noexcept {
  return Sequence_Parser<Ps...>{syntag, std::forward<Ps>(ps)...};
}

//! Generate a HOIST Sequence_Parser
template <typename... Ps>
static constexpr Sequence_Parser<Ps...> h_seq(Ps &&... ps) noexcept {
  return Sequence_Parser<Ps...>{Syntax_Tags::HOIST, std::forward<Ps>(ps)...};
}

//! If the first parser matches, all specified parsers must match
template <typename P0, typename... Ps> class Sequence_If_Parser {
public:
  Sequence_If_Parser(Sequence_If_Parser const &) = default;
  constexpr explicit Sequence_If_Parser(int const syntag, P0 &&p0,
                                        Ps &&... ps) noexcept
      : syntag_{syntag}, parser0_{p0}, rest_{std::forward<Ps>(ps)...} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root(syntag_);
    auto attach_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (pt) {
        hoist_back(root, std::move(pt));
      }
      return match;
    };
    auto fold = [&attach_if](auto... as) constexpr {
      return FLPR::details_::and_fold(attach_if, as...);
    };

    if (attach_if(parser0_)) {
      bool const match = std::apply(fold, rest_);
      if (!match) {
        std::cerr << "Unrecognized statement\n";
        state.ss->print_me(std::cerr, false) << "\nwhile parsing ";
        FLPR::Syntax_Tags::print(std::cerr, syntag_) << std::endl;
        return PP_Result{};
      }
      if (root)
        cover_branches(*root);
      return PP_Result{std::move(root), match};
    }
    return PP_Result{};
  }

private:
  int const syntag_;
  P0 parser0_;
  std::tuple<Ps...> const rest_;
};

//! Generate a Sequence_If_Parser
template <typename... Ps>
static constexpr Sequence_If_Parser<Ps...> seq_if(int const syntag,
                                                  Ps &&... ps) noexcept {
  return Sequence_If_Parser<Ps...>{syntag, std::forward<Ps>(ps)...};
}

//! Generate a HOIST Sequence_If_Parser
template <typename... Ps>
static constexpr Sequence_If_Parser<Ps...> h_seq_if(Ps &&... ps) noexcept {
  return Sequence_If_Parser<Ps...>{Syntax_Tags::HOIST, std::forward<Ps>(ps)...};
}

//! Kleene star: Zero or more matches of the parser
template <typename P> class Star_Parser {
public:
  Star_Parser(Star_Parser const &) = default;
  constexpr explicit Star_Parser(P &&p) noexcept
      : parser_{std::forward<P>(p)} {}
  PP_Result operator()(State &state) const noexcept {
    Prgm_Tree root(Syntax_Tags::HOIST);
    auto attach_if = [&state, &root ](auto const &p) constexpr {
      auto [pt, match] = p(state);
      if (pt) {
        hoist_back(root, std::move(pt));
      }
      return match;
    };
    while (state.ss && attach_if(parser_))
      ;
    if (root)
      cover_branches(*root);
    // The list root may have no children, but this always matches
    return PP_Result{std::move(root), true};
  }

private:
  P const parser_;
};

//! Generate a Star_Parser
template <typename P> static constexpr Star_Parser<P> star(P &&p) noexcept {
  return Star_Parser<P>{std::forward<P>(p)};
}

//! Return the Stmt_Tree generated by a function, match means tree is good
class Statement_Parser {
public:
  using parser_function = FLPR::Stmt::Stmt_Tree (*)(FLPR::TT_Stream &ts);
  Statement_Parser(Statement_Parser const &) = default;
  constexpr explicit Statement_Parser(parser_function f) noexcept : f_{f} {}
  PP_Result operator()(State &state) const noexcept {
    FLPR::TT_Stream tts(*(state.ss));
    FLPR::Stmt::Stmt_Tree st = f_(tts);
    if (!st)
      return PP_Result{};
    int const tag = (*st)->syntag;
    FLPR::LL_STMT_SEQ::iterator ll_stmt_it{state.ss};
    state.ss.advance();
    ll_stmt_it->set_stmt_tree(std::move(st));
    return PP_Result{Prgm_Tree{tag, ll_stmt_it}, true};
  }

private:
  parser_function const f_;
};

//! Generate a Statement_Parser
static constexpr auto stmt(typename Statement_Parser::parser_function f) {
  return Statement_Parser{f};
};

//! "Tag" a Prgm_Tree with a new root
template <typename P> class Tag_Parser {
public:
  Tag_Parser(Tag_Parser const &) = default;
  constexpr explicit Tag_Parser(int const syntag, P &&p) noexcept
      : syntag_{syntag}, parser_{std::forward<P>(p)} {}
  PP_Result operator()(State &state) const noexcept {
    PP_Result res = parser_(state);
    if (res.parse_tree) {
      Prgm_Tree new_root{syntag_};
      hoist_back(new_root, std::move(res.parse_tree));
      if (new_root)
        cover_branches(*new_root);
      return PP_Result{std::move(new_root), res.match};
    }
    return res;
  }

private:
  int const syntag_;
  P const parser_;
};

//! Generate a Tag_Parser
template <typename P>
static constexpr Tag_Parser<P> tag_if(int const syntag, P &&p) noexcept {
  return Tag_Parser<P>{syntag, std::forward<P>(p)};
}

/**************************** DO-CONSTRUCT PARSERS ***************************/

/* This is a special parser developed to handle the legacy
   nonblock-do-construct, which was deprecated in F08, and removed in F18.
   While a loop with a nonlabel-do-stmt is easy to parse using recursive descent
   and combinators, it is a different story for label-do-stmt.  The tricky part
   is that you cannot determine what type of construct you have until you reach
   the labeled terminating statement.  Here are examples of each construct that
   starts with label-do-stmt:

   block-do:label-do-stmt:

       DO 100 loop-control
          block => [execution-part-construct]...
   100 CONTINUE or ENDDO

   nonblock-do:action-term-do-construct:

       DO 100 loop-control
           [execution-part-construct]...
   100 do-term-action-stmt => action-stmt, except for CONTINUE and nonsense.

   nonblock-do:outer-shared-do-construct:

       DO 100 i=1,2
         do-body => [execution-part-construct]...
         DO 100 loop-control
           do-body => [execution-part-construct]...
   100 do-term-shared-stmt => action-stmt, incl CONTINUE, excl nonsense

   (nonsense do-term-* statements would be something like RETURN).

   To differentiate between these forms, we need to:

      1) look for nested label-do-stmts that share a label
            => nonblock outer-shared-do-construct
      2) look for the labeled stmt being CONTINUE
            => standard F18 do-construct
      3) otherwise, the labeled stmt should be an action-stmt
            => nonblock action-term-do-constuct

   Because the label-do-stmt is a deprecated form of do-construct in F18,
   we are electing to NOT adopt the F08 partitioning of do-construct into
   block-do-construct and nonblock-do-construct, but to introduce
   nonblock-do-construct as a peer entity to do-construct (which only has
   block forms).
*/

//! Return the Stmt_Tree generated by Stmt::do_stmt, match means tree is good
/*! This is special because a label-do-stmt needs to add its terminating label
    to the State::label_stack. */
class Do_Stmt_Parser {
public:
  constexpr Do_Stmt_Parser() = default;
  PP_Result operator()(State &state) const noexcept {
    FLPR::TT_Stream tts(*(state.ss));
    FLPR::Stmt::Stmt_Tree st = FLPR::Stmt::do_stmt(tts);
    if (!st)
      return PP_Result{};
    int const tag = (*st)->syntag;
    FLPR::LL_STMT_SEQ::iterator ll_stmt_it{state.ss};
    state.ss.advance();
    /* if this is a label-do-stmt, add the label to the stack */
    const int l = FLPR::Stmt::get_label_do_label(st);
    if (l > 0)
      state.do_label_stack.push_back(l);
    ll_stmt_it->set_stmt_tree(std::move(st));
    return PP_Result{Prgm_Tree{tag, ll_stmt_it}, true};
  }
};

//! Generate a Do_Stmt_Parser
static constexpr auto do_stmt() { return Do_Stmt_Parser{}; }

//! Match an end-do
class End_Do_Parser {
public:
  constexpr End_Do_Parser() = default;
  PP_Result operator()(State &state) const noexcept {

    FLPR::TT_Stream tts(*(state.ss));

    /* Without a label, the only end-do this can be is a end-do-statment */
    if (!state.ss->has_label()) {
      FLPR::Stmt::Stmt_Tree st = FLPR::Stmt::end_do_stmt(tts);
      if (!st)
        return PP_Result{}; // wasn't end-do-stmt, so match fails
      int const tag = (*st)->syntag;
      FLPR::LL_STMT_SEQ::iterator ll_stmt_it{state.ss};
      state.ss.advance();
      ll_stmt_it->set_stmt_tree(std::move(st));
      return PP_Result{Prgm_Tree{tag, ll_stmt_it}, true}; // matched end-do
    }

    /* So, we've got a label. Let N be the number of instances of this
       label on the stack. This means, evaluated in order:
       if N > 1 && and is action statement  => outer-shared-do-construct
       N == 1 && end-do => block-do-construct
       N == 1 && action-stmt => action-term-do-construct
    */

    int const label{state.ss->label()};
    int const NL = std::count(state.do_label_stack.begin(),
                              state.do_label_stack.end(), label);
    if (NL == 0)
      return PP_Result{};

    if (NL > 1) {
      /* this is a do-term-shared-stmt, and the first N-1 label-do-stmts that we
         are closing off are inner-shared-do-constructs and the last one is an
         outer-shared-do-construct form of a nonblock-do-construct. */
      FLPR::Stmt::Stmt_Tree st = FLPR::Stmt::action_stmt(tts);
      if (!st)
        return PP_Result{};
      int const tag = (*st)->syntag;
      FLPR::LL_STMT_SEQ::iterator ll_stmt_it{state.ss};

      /* Mark the top label as matched */
      if (label == state.do_label_stack.back()) {
        state.do_label_stack.pop_back();
      }

      /* We don't advance the stream if the next do label that we are looking
         for matches the label of the current statement.  This allows multiple
         label-do-stmt to terminate on the same label, as this statement will
         still be in the stream for the next do-construct termination.  */
      if (state.do_label_stack.empty() ||
          label != state.do_label_stack.back()) {
        state.ss.advance();
      }
      ll_stmt_it->set_stmt_tree(std::move(st));
      return PP_Result{Prgm_Tree{tag, ll_stmt_it}, true};
    }
    /* we're left with this being either a label-do-stmt form of
       block-do-construct, or an action-term-do-construct form of a
       nonblock-do-construct. */

    FLPR::Stmt::Stmt_Tree st = FLPR::Stmt::end_do(tts);
    if (!st) {
      /* It isn't end-do-stmt or continue-stmt, so this should be an
         do-term-action-stmt, which closes an action-term-do-construct */
      st = FLPR::Stmt::action_stmt(tts);

    } /* else: end-do => block-do-construct */

    if (!st)
      return PP_Result{};
    int const tag = (*st)->syntag;
    FLPR::LL_STMT_SEQ::iterator ll_stmt_it{state.ss};
    state.ss.advance();

    assert(state.do_label_stack.back() == label);
    state.do_label_stack.pop_back();

    ll_stmt_it->set_stmt_tree(std::move(st));

    return PP_Result{Prgm_Tree{tag, ll_stmt_it}, true};
  }
};

//! Generate a End_Do_Parser
static constexpr auto end_do() { return End_Do_Parser{}; }

#endif
