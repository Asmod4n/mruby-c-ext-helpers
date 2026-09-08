#pragma once

/* MSVC reports __cplusplus as 199711L unless /Zc:__cplusplus is set, so the
   real language level has to be read from _MSVC_LANG there. */
#if (defined(_MSVC_LANG) && _MSVC_LANG < 201703L) || \
    (!defined(_MSVC_LANG) && __cplusplus < 201703L)
#error "num_helpers.hpp requires C++17 -- add the flag to your gem AND note that spec.cxx.flags do not propagate to dependent gems"
#endif

#include <type_traits>
#include <limits>
#include <cstdint>
#include <mruby.h>
#include <mruby/numeric.h>
#include <mruby/value.h>

namespace mrbcpp::number_converter {
  template <typename T>
  constexpr bool type_fits_fixnum() {
    if constexpr (std::is_signed_v<T>) {
      return (std::numeric_limits<T>::lowest)() >= MRB_FIXNUM_MIN &&
             (std::numeric_limits<T>::max)()    <= MRB_FIXNUM_MAX;
    } else {
      using U = std::make_unsigned_t<mrb_int>;
      return (std::numeric_limits<T>::max)() <= static_cast<U>(MRB_FIXNUM_MAX);
    }
  }

  template <typename T>
  constexpr bool type_fits_int() {
    if constexpr (std::is_signed_v<T>) {
      return (std::numeric_limits<T>::lowest)() >= MRB_INT_MIN &&
             (std::numeric_limits<T>::max)()    <= MRB_INT_MAX;
    } else {
      using U = std::make_unsigned_t<mrb_int>;
      return (std::numeric_limits<T>::max)() <= static_cast<U>(MRB_INT_MAX);
    }
  }

}

template <typename T>
MRB_API mrb_value mrb_convert_number(mrb_state* mrb, T value) {
  using namespace mrbcpp::number_converter;

  // ------------------------------------------------------------
  // ENUMS
  // ------------------------------------------------------------
  if constexpr (std::is_enum_v<T>) {
    using U = std::underlying_type_t<T>;
    return mrb_convert_number(mrb, static_cast<U>(value));
  }

  // ------------------------------------------------------------
  // FLOATING POINT
  // ------------------------------------------------------------
  else if constexpr (std::is_floating_point_v<T>) {
#ifndef MRB_NO_FLOAT
if constexpr ((std::numeric_limits<T>::lowest)() >= (std::numeric_limits<mrb_float>::lowest)() &&
                  (std::numeric_limits<T>::max)()    <= (std::numeric_limits<mrb_float>::max)()) {
      return mrb_float_value(mrb, static_cast<mrb_float>(value));
    } else {
      // Type doesn't fit statically, check runtime value
      if (isfinite(value) &&
          value >= (std::numeric_limits<mrb_float>::lowest)() &&
          value <= (std::numeric_limits<mrb_float>::max)()) {
        return mrb_float_value(mrb, static_cast<mrb_float>(value));
      } else {
        mrb_raise(mrb, E_RANGE_ERROR, "Float too large for mrb_float");
      }
    }
#else
    mrb_raise(mrb, E_TYPE_ERROR, "Float support disabled");
#endif
  }

  // ------------------------------------------------------------
  // INTEGRAL TYPES
  // ------------------------------------------------------------
  else if constexpr (std::is_integral_v<T>) {

    // Fits fixnum?
    if constexpr (type_fits_fixnum<T>()) {
      return mrb_fixnum_value(static_cast<mrb_int>(value));
    }

    // Fits mrb_int?
    else if constexpr (type_fits_int<T>()) {
      return mrb_int_value(mrb, static_cast<mrb_int>(value));
    }

    // Signed overflow
    if constexpr (std::is_signed_v<T>) {
      if (value >= MRB_INT_MIN && value <= MRB_INT_MAX) {
        return mrb_int_value(mrb, static_cast<mrb_int>(value));
      }
      return mrb_int64_value(mrb, static_cast<int64_t>(value));
    }

    // Unsigned overflow
    else {
      if (value <= static_cast<std::make_unsigned_t<mrb_int>>(MRB_INT_MAX)) {
        return mrb_int_value(mrb, static_cast<mrb_int>(value));
      }
      return mrb_uint64_value(mrb, static_cast<uint64_t>(value));
    }
  }

  // ------------------------------------------------------------
  // FALLBACK
  // ------------------------------------------------------------
  mrb_raise(mrb, E_TYPE_ERROR, "Unsupported numeric type");
}
