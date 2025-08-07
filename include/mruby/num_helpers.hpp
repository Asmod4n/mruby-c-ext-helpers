#pragma once

#include <mruby.h>
#include <limits>
#include <type_traits>
#include <cstdint>

MRB_BEGIN_DECL
#include <mruby/internal.h>
MRB_END_DECL

namespace mrbcpp::number_converter {
  template<typename From, typename To>
  constexpr static bool fits_in() {
    using FL = std::numeric_limits<From>;
    using TL = std::numeric_limits<To>;
    if constexpr (std::is_signed_v<From> == std::is_signed_v<To>) {
      return FL::lowest() >= TL::lowest() && FL::max() <= TL::max();
    } else if constexpr (std::is_signed_v<From>) {
      return FL::lowest() >= 0 &&
            static_cast<std::make_unsigned_t<From>>(FL::max()) <= TL::max();
    } else {
      return FL::max() <= static_cast<std::make_unsigned_t<To>>(TL::max());
    }
  }
}

template <typename T>
constexpr MRB_API mrb_value mrb_convert_number(mrb_state* mrb, T value) {
  using limits = std::numeric_limits<T>;
  if constexpr (std::is_floating_point_v<T>) {
#ifndef MRB_NO_FLOAT
    if constexpr (limits::lowest() >= std::numeric_limits<mrb_float>::lowest() &&
                  limits::max()    <= std::numeric_limits<mrb_float>::max()) {
      return mrb_float_value(mrb, static_cast<mrb_float>(value));
    } else {
      mrb_raise(mrb, E_RANGE_ERROR, "Float too large for mrb_float");
    }
#else
    mrb_raise(mrb, E_TYPE_ERROR, "Float support has been disabled in this mruby installation");
#endif
  } else if constexpr (std::is_integral_v<T>) {
    if constexpr (mrbcpp::number_converter::fits_in<T, mrb_int>()) {
      return mrb_fixnum_value(static_cast<mrb_int>(value));
    }
#ifdef MRB_USE_BIGINT
    if constexpr (std::is_signed_v<T>) {
      return mrb_bint_new_int64(mrb, static_cast<int64_t>(value));
    } else {
      return mrb_bint_new_uint64(mrb, static_cast<uint64_t>(value));
    }
#else
    mrb_raise(mrb, E_RANGE_ERROR,
              "Integer too large for mrb_int and MRB_USE_BIGINT not enabled");
#endif
  } else {
    mrb_raise(mrb, E_TYPE_ERROR, "Unsupported numeric type");
  }
}
