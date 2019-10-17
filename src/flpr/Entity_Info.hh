/*
   Copyright (c) 2019, Triad National Security, LLC. All rights reserved.

   This is open source software; you can redistribute it and/or modify it
   under the terms of the BSD-3 License. If software is modified to produce
   derivative works, such modified software should be clearly marked, so as
   not to confuse it with the version available from LANL. Full text of the
   BSD-3 License can be found in the LICENSE file of the repository.
*/

/*!
  \file Entity_Info

  Define attributes for an data entity/object
*/


namespace FLPR {

class Entity_Info {
public:
  constexpr bool is_allocatable() const noexcept { return allocatable_; }
  constexpr void is_allocatable(const bool value) noexcept { allocatable_ = value; }
  constexpr bool is_asynchronous() const noexcept { return asynchronous_; }
  constexpr void is_asynchronous(const bool value) noexcept { asynchronous_ = value; }
  constexpr bool is_contiguous() const noexcept { return contiguous_; }
  constexpr void is_contiguous(const bool value) noexcept { contiguous_ = value; }
  constexpr bool is_external() const noexcept { return external_; }
  constexpr void is_external(const bool value) noexcept { external_ = value; }
  constexpr bool is_intrinsic() const noexcept { return intrinsic_; }
  constexpr void is_intrinsic(const bool value) noexcept { intrinsic_ = value; }
  constexpr bool is_optional() const noexcept { return optional_; }
  constexpr void is_optional(const bool value) noexcept { optional_ = value; }
  constexpr bool is_parameter() const noexcept { return parameter_; }
  constexpr void is_parameter(const bool value) noexcept { parameter_ = value; }
  constexpr bool is_pointer() const noexcept { return pointer_; }
  constexpr void is_pointer(const bool value) noexcept { pointer_ = value; }
  constexpr bool is_protected() const noexcept { return protected_; }
  constexpr void is_protected(const bool value) noexcept { protected_ = value; }
  constexpr bool is_save() const noexcept { return save_; }
  constexpr void is_save(const bool value) noexcept { save_ = value; }
  constexpr bool is_target() const noexcept { return target_; }
  constexpr void is_target(const bool value) noexcept { target_ = value; }
  constexpr bool is_value() const noexcept { return value_; }
  constexpr void is_value(const bool value) noexcept { value_ = value; }
  constexpr bool is_volatile() const noexcept { return volatile_; }
  constexpr void is_volatile(const bool value) noexcept { volatile_ = value; }
private:
  bool allocatable_ = false;
  bool asynchronous_ = false;
  bool contiguous_ = false;
  bool external_ = false;
  bool intrinsic_ = false;
  bool optional_ = false;
  bool parameter_ = false;
  bool pointer_ = false;
  bool protected_ = false;
  bool save_ = false;
  bool target_ = false;
  bool value_ = false;
  bool volatile_ = false;
  
};

}
