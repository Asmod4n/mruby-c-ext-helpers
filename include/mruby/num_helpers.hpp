#pragma once
#include <mruby.h>
MRB_BEGIN_DECL
#include <mruby/internal.h>
MRB_END_DECL
#include <limits>

template <typename T>
MRB_API mrb_value
mrb_convert_number(mrb_state* mrb, T value)
{
  constexpr auto fits_in = []<typename From, typename To>() constexpr -> bool {
    using FromLimits = std::numeric_limits<From>;
    using ToLimits   = std::numeric_limits<To>;

    if constexpr (std::is_signed_v<From> == std::is_signed_v<To>) {
      return FromLimits::lowest() >= ToLimits::lowest() &&
             FromLimits::max()    <= ToLimits::max();
    }
    else if constexpr (std::is_signed_v<From> && !std::is_signed_v<To>) {
      // signed -> unsigned
      return FromLimits::lowest() >= 0 &&
             static_cast<std::make_unsigned_t<From>>(FromLimits::max()) <= ToLimits::max();
    }
    else if constexpr (!std::is_signed_v<From> && std::is_signed_v<To>) {
      // unsigned -> signed
      return FromLimits::max() <= static_cast<std::make_unsigned_t<To>>(ToLimits::max());
    }
    else {
      static_assert(!sizeof(From), "Unexpected signed/unsigned combo in fits_in");
    }
  };

  using limits = std::numeric_limits<T>;

  // Handle floating-point types
  if constexpr (std::is_floating_point_v<T>) {
#ifndef MRB_NO_FLOAT
    if constexpr (limits::lowest() >= std::numeric_limits<mrb_float>::lowest() &&
                  limits::max() <= std::numeric_limits<mrb_float>::max()) {
      return mrb_float_value(mrb, static_cast<mrb_float>(value));
    } else {
      mrb_raise(mrb, E_RANGE_ERROR, "Float too large for mrb_float");
    }
#else
    mrb_raise(mrb, E_TYPE_ERROR, "Float support is disabled");
#endif
  }
  else if constexpr (std::is_integral_v<T>) {
    if constexpr (fits_in.template operator()<T, mrb_int>()) {
      return mrb_fixnum_value(static_cast<mrb_int>(value));
    }

#ifdef MRB_USE_BIGINT
    constexpr bool is_signed = std::is_signed_v<T>;
    if constexpr (is_signed) {
      return mrb_bint_new_int64(mrb, static_cast<int64_t>(value));
    } else {
      return mrb_bint_new_uint64(mrb, static_cast<uint64_t>(value));
    }
#else
    mrb_raise(mrb, E_RANGE_ERROR, "Integer too large for mrb_int and MRB_USE_BIGINT not enabled");
#endif
  }
  else {
    mrb_raise(mrb, E_TYPE_ERROR, "Unsupported numeric type");
  }
}
