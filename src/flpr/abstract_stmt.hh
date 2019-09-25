
/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file abstract_stmt.hh
*/
#ifndef FLPR_ABSTRACT_STMT_HH
#define FLPR_ABSTRACT_STMT_HH 1

#include "flpr/Stmt_Tree.hh"
#include "flpr/TT_Stream.hh"

namespace FLPR {


//! Namespace containing Stmt_Tree->AST converters
namespace AST {

//! The contents of each \c AS_Tree node
class AST_Node_Data {
public:
  using st_cursor = FLPR::Stmt::Stmt_Tree::const_cursor_t;
  using value_type = st_cursor::value_type;
  using reference = st_cursor::reference;
  using pointer = st_cursor::pointer;
public:
  AST_Node_Data() : syntag_(Syntax_Tags::UNKNOWN) {}
  AST_Node_Data(int syntag) : syntag_(syntag) {}
  AST_Node_Data(int syntag, st_cursor const &stc) : syntag_(syntag),stc_(stc) {}
  constexpr int syntag() const { return syntag_; }
  constexpr bool has_stmt_cursor() const { return stc_ == true; }
  st_cursor stmt_cursor() const { return stc_; }
private:
  int syntag_;
  st_cursor stc_;
};

using AS_Tree = Tree<AST_Node_Data>;


std::ostream &operator<<(std::ostream &os, AST_Node_Data const &nd);

using st_cursor = AST_Node_Data::st_cursor;

//! Dispatch to a particular parser given the SG_* tag at stc.
AS_Tree stmt_dispatch(st_cursor stc);

/*! \defgroup Converters for complete Fortran statements
  @{ */
AS_Tree access_stmt(st_cursor stc);
AS_Tree action_stmt(st_cursor stc);
AS_Tree allocatable_stmt(st_cursor stc);
AS_Tree allocate_stmt(st_cursor stc);
AS_Tree arithmetic_if_stmt(st_cursor stc);
AS_Tree assignment_stmt(st_cursor stc);
AS_Tree associate_stmt(st_cursor stc);
AS_Tree asynchronous_stmt(st_cursor stc);
AS_Tree backspace_stmt(st_cursor stc);
AS_Tree bind_stmt(st_cursor stc);
AS_Tree binding_private_stmt(st_cursor stc);
AS_Tree block_stmt(st_cursor stc);
AS_Tree call_stmt(st_cursor stc);
AS_Tree case_stmt(st_cursor stc);
AS_Tree close_stmt(st_cursor stc);
AS_Tree codimension_stmt(st_cursor stc);
AS_Tree common_stmt(st_cursor stc);
AS_Tree component_def_stmt(st_cursor stc);
AS_Tree computed_goto_stmt(st_cursor stc);
AS_Tree contains_stmt(st_cursor stc);
AS_Tree continue_stmt(st_cursor stc);
AS_Tree cycle_stmt(st_cursor stc);
AS_Tree data_component_def_stmt(st_cursor stc);
AS_Tree data_stmt(st_cursor stc);
AS_Tree deallocate_stmt(st_cursor stc);
AS_Tree derived_type_stmt(st_cursor stc);
AS_Tree dimension_stmt(st_cursor stc);
AS_Tree do_stmt(st_cursor stc);
AS_Tree else_if_stmt(st_cursor stc);
AS_Tree else_stmt(st_cursor stc);
AS_Tree elsewhere_stmt(st_cursor stc);
AS_Tree end_associate_stmt(st_cursor stc);
AS_Tree end_block_stmt(st_cursor stc);
AS_Tree end_do(st_cursor stc);
AS_Tree end_do_stmt(st_cursor stc);
AS_Tree end_enum_stmt(st_cursor stc);
AS_Tree end_forall_stmt(st_cursor stc);
AS_Tree end_function_stmt(st_cursor stc);
AS_Tree end_if_stmt(st_cursor stc);
AS_Tree end_interface_stmt(st_cursor stc);
AS_Tree end_module_stmt(st_cursor stc);
AS_Tree end_mp_subprogram_stmt(st_cursor stc);
AS_Tree end_program_stmt(st_cursor stc);
AS_Tree end_select_stmt(st_cursor stc);
AS_Tree end_select_rank_stmt(st_cursor stc);
AS_Tree end_select_type_stmt(st_cursor stc);
AS_Tree end_submodule_stmt(st_cursor stc);
AS_Tree end_subroutine_stmt(st_cursor stc);
AS_Tree end_type_stmt(st_cursor stc);
AS_Tree end_where_stmt(st_cursor stc);
AS_Tree endfile_stmt(st_cursor stc);
AS_Tree entry_stmt(st_cursor stc);
AS_Tree enum_def_stmt(st_cursor stc);
AS_Tree enumerator_def_stmt(st_cursor stc);
AS_Tree equivalence_stmt(st_cursor stc);
AS_Tree error_stop_stmt(st_cursor stc);
AS_Tree event_post_stmt(st_cursor stc);
AS_Tree event_wait_stmt(st_cursor stc);
AS_Tree exit_stmt(st_cursor stc);
AS_Tree external_stmt(st_cursor stc);
AS_Tree fail_image_stmt(st_cursor stc);
AS_Tree final_procedure_stmt(st_cursor stc);
AS_Tree flush_stmt(st_cursor stc);
AS_Tree forall_assignment_stmt(st_cursor stc);
AS_Tree forall_construct_stmt(st_cursor stc);
AS_Tree forall_stmt(st_cursor stc);
AS_Tree form_team_stmt(st_cursor stc);
AS_Tree format_stmt(st_cursor stc);
AS_Tree function_stmt(st_cursor stc);
AS_Tree generic_stmt(st_cursor stc);
AS_Tree goto_stmt(st_cursor stc);
AS_Tree if_stmt(st_cursor stc);
AS_Tree if_then_stmt(st_cursor stc);
AS_Tree implicit_stmt(st_cursor stc);
AS_Tree import_stmt(st_cursor stc);
AS_Tree inquire_stmt(st_cursor stc);
AS_Tree intent_stmt(st_cursor stc);
AS_Tree interface_stmt(st_cursor stc);
AS_Tree intrinsic_stmt(st_cursor stc);
AS_Tree label_do_stmt(st_cursor stc);
AS_Tree lock_stmt(st_cursor stc);
AS_Tree macro_stmt(st_cursor stc);
AS_Tree masked_elsewhere_stmt(st_cursor stc);
AS_Tree module_stmt(st_cursor stc);
AS_Tree mp_subprogram_stmt(st_cursor stc);
AS_Tree namelist_stmt(st_cursor stc);
AS_Tree nonlabel_do_stmt(st_cursor stc);
AS_Tree nullify_stmt(st_cursor stc);
AS_Tree open_stmt(st_cursor stc);
AS_Tree optional_stmt(st_cursor stc);
AS_Tree other_specification_stmt(st_cursor stc);
AS_Tree parameter_stmt(st_cursor stc);
AS_Tree pointer_assignment_stmt(st_cursor stc);
AS_Tree pointer_stmt(st_cursor stc);
AS_Tree print_stmt(st_cursor stc);
AS_Tree private_components_stmt(st_cursor stc);
AS_Tree private_or_sequence(st_cursor stc);
AS_Tree proc_component_def_stmt(st_cursor stc);
AS_Tree procedure_declaration_stmt(st_cursor stc);
AS_Tree procedure_stmt(st_cursor stc);
AS_Tree program_stmt(st_cursor stc);
AS_Tree protected_stmt(st_cursor stc);
AS_Tree read_stmt(st_cursor stc);
AS_Tree return_stmt(st_cursor stc);
AS_Tree rewind_stmt(st_cursor stc);
AS_Tree save_stmt(st_cursor stc);
AS_Tree select_case_stmt(st_cursor stc);
AS_Tree select_rank_stmt(st_cursor stc);
AS_Tree select_rank_case_stmt(st_cursor stc);
AS_Tree select_type_stmt(st_cursor stc);
AS_Tree sequence_stmt(st_cursor stc);
AS_Tree stop_stmt(st_cursor stc);
AS_Tree submodule_stmt(st_cursor stc);
AS_Tree subroutine_stmt(st_cursor stc);
AS_Tree sync_all_stmt(st_cursor stc);
AS_Tree sync_images_stmt(st_cursor stc);
AS_Tree sync_memory_stmt(st_cursor stc);
AS_Tree sync_team_stmt(st_cursor stc);
AS_Tree target_stmt(st_cursor stc);
AS_Tree type_bound_generic_stmt(st_cursor stc);
AS_Tree type_bound_procedure_stmt(st_cursor stc);
AS_Tree type_declaration_stmt(st_cursor stc);
AS_Tree type_guard_stmt(st_cursor stc);
AS_Tree type_param_def_stmt(st_cursor stc);
AS_Tree unlock_stmt(st_cursor stc);
AS_Tree use_stmt(st_cursor stc);
AS_Tree value_stmt(st_cursor stc);
AS_Tree volatile_stmt(st_cursor stc);
AS_Tree wait_stmt(st_cursor stc);
AS_Tree where_construct_stmt(st_cursor stc);
AS_Tree where_stmt(st_cursor stc);
AS_Tree write_stmt(st_cursor stc);
/*! @} */

/*! \defgroup converters for portions of Fortran statements
  @{ */
AS_Tree actual_arg_spec(st_cursor stc);
AS_Tree allocatable_decl(st_cursor stc);
AS_Tree allocate_coarray_spec(st_cursor stc);
AS_Tree allocate_coshape_spec(st_cursor stc);
AS_Tree allocation(st_cursor stc);
AS_Tree array_element(st_cursor stc);
AS_Tree array_spec(st_cursor stc);
AS_Tree association(st_cursor stc);
AS_Tree assumed_implied_spec(st_cursor stc);
AS_Tree assumed_rank_spec(st_cursor stc);
AS_Tree assumed_shape_spec(st_cursor stc);
AS_Tree assumed_size_spec(st_cursor stc);
AS_Tree case_value_range(st_cursor stc);
AS_Tree char_selector(st_cursor stc);
AS_Tree coarray_spec(st_cursor stc);
AS_Tree component_initialization(st_cursor stc);
AS_Tree concurrent_control(st_cursor stc);
AS_Tree concurrent_header(st_cursor stc);
AS_Tree concurrent_locality(st_cursor stc);
AS_Tree data_ref(st_cursor stc);
AS_Tree data_stmt_object(st_cursor stc);
AS_Tree data_stmt_set(st_cursor stc);
AS_Tree data_stmt_value(st_cursor stc);
AS_Tree declaration_type_spec(st_cursor stc);
AS_Tree declaration_type_spec_no_kind(st_cursor stc);
AS_Tree deferred_shape_spec(st_cursor stc);
AS_Tree defined_io_generic_spec(st_cursor stc);
AS_Tree derived_type_spec(st_cursor stc);
AS_Tree designator(st_cursor stc);
AS_Tree dummy_arg(st_cursor stc);
AS_Tree explicit_coshape_spec(st_cursor stc);
AS_Tree explicit_shape_spec(st_cursor stc);
AS_Tree expr(st_cursor stc);
AS_Tree extended_intrinsic_op(st_cursor stc);
AS_Tree function_reference(st_cursor stc);
AS_Tree generic_spec(st_cursor stc);
AS_Tree image_selector(st_cursor stc);
AS_Tree image_selector_spec(st_cursor stc);
AS_Tree implicit_none_spec(st_cursor stc);
AS_Tree implied_shape_or_assumed_size_spec(st_cursor stc);
AS_Tree implied_shape_spec(st_cursor stc);
AS_Tree initialization(st_cursor stc);
AS_Tree input_item(st_cursor stc);
AS_Tree int_constant_expr(st_cursor stc);
AS_Tree int_expr(st_cursor stc);
AS_Tree integer_type_spec(st_cursor stc);
AS_Tree intent_spec(st_cursor stc);
AS_Tree intrinsic_operator(st_cursor stc);
AS_Tree intrinsic_type_spec(st_cursor stc);
AS_Tree intrinsic_type_spec_no_kind(st_cursor stc);
AS_Tree io_implied_do(st_cursor stc);
AS_Tree io_implied_do_control(st_cursor stc);
AS_Tree io_implied_do_object(st_cursor stc);
AS_Tree kind_selector(st_cursor stc);
AS_Tree label(st_cursor stc);
AS_Tree language_binding_spec(st_cursor stc);
AS_Tree length_selector(st_cursor stc);
AS_Tree letter_spec(st_cursor stc);
AS_Tree locality_spec(st_cursor stc);
AS_Tree logical_expr(st_cursor stc);
AS_Tree logical_literal_constant(st_cursor stc);
AS_Tree loop_control(st_cursor stc);
AS_Tree lower_bound_expr(st_cursor stc);
AS_Tree mult_op(st_cursor stc);
AS_Tree output_item(st_cursor stc);
AS_Tree parent_string(st_cursor stc);
AS_Tree part_ref(st_cursor stc);
AS_Tree pointer_object(st_cursor stc);
AS_Tree prefix(st_cursor stc);
AS_Tree prefix_spec(st_cursor stc);
AS_Tree proc_component_attr_spec(st_cursor stc);
AS_Tree proc_component_ref(st_cursor stc);
AS_Tree proc_decl(st_cursor stc);
AS_Tree proc_interface(st_cursor stc);
AS_Tree proc_language_binding_spec(st_cursor stc);
AS_Tree proc_pointer_init(st_cursor stc);
AS_Tree proc_pointer_object(st_cursor stc);
AS_Tree proc_target(st_cursor stc);
AS_Tree procedure_designator(st_cursor stc);
AS_Tree rel_op(st_cursor stc);
AS_Tree rename(st_cursor stc);
AS_Tree saved_entity(st_cursor stc);
AS_Tree section_subscript(st_cursor stc);
AS_Tree selector(st_cursor stc);
AS_Tree sign(st_cursor stc);
AS_Tree structure_component(st_cursor stc);
AS_Tree structure_constructor(st_cursor stc);
AS_Tree substring(st_cursor stc);
AS_Tree substring_range(st_cursor stc);
AS_Tree suffix(st_cursor stc);
AS_Tree sync_stat(st_cursor stc);
AS_Tree type_attr_spec(st_cursor stc);
AS_Tree type_bound_proc_binding(st_cursor stc);
AS_Tree type_param_spec(st_cursor stc);
AS_Tree type_param_value(st_cursor stc);
AS_Tree type_spec(st_cursor stc);
AS_Tree upper_bound_expr(st_cursor stc);
AS_Tree variable(st_cursor stc);
AS_Tree wait_spec(st_cursor stc);

AS_Tree consume_parens(st_cursor stc);

/*! @} */

} // namespace AST
} // namespace FLPR

#endif
