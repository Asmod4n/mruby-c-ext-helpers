#pragma once
#include <type_traits>
#include <limits>
#include <cstdint>
#include <mruby.h>
#include <mruby/numeric.h>
#include <mruby/value.h>
MRB_BEGIN_DECL
#include <mruby/internal.h>
MRB_END_DECL

namespace mrbcpp::number_converter {
  // Compile-time: does the full TYPE range fit into fixnum?
  template <typename T>
  constexpr bool type_fits_fixnum() {
    return std::numeric_limits<T>::lowest() >= MRB_FIXNUM_MIN &&
           std::numeric_limits<T>::max()    <= MRB_FIXNUM_MAX;
  }

  // Compile-time: does the full TYPE range fit into mrb_int?
  template <typename T>
  constexpr bool type_fits_int() {
    return std::numeric_limits<T>::lowest() >= MRB_INT_MIN &&
           std::numeric_limits<T>::max()    <= MRB_INT_MAX;
  }

  // Compile-time: is the TYPE wider than mrb_int and can overflow it?
  template <typename T>
  constexpr bool type_needs_bint() {
    return !type_fits_int<T>();
  }

#if defined(__SIZEOF_INT128__)
  template <typename T> struct is_int128   : std::false_type {};
  template <typename T> struct is_uint128  : std::false_type {};
  template <> struct is_int128<__int128>             : std::true_type {};
  template <> struct is_uint128<unsigned __int128>   : std::true_type {};
#endif


#if defined(__SIZEOF_INT128__) && defined(MRB_USE_BIGINT)
  static inline mrb_value mrb_bint_new_uint128(mrb_state* mrb, unsigned __int128 u) {
    uint64_t lo = static_cast<uint64_t>(u);
    uint64_t hi = static_cast<uint64_t>(u >> 64);

    mrb_value v_lo = mrb_bint_new_uint64(mrb, lo);
    if (hi == 0) return v_lo;

    mrb_value v_hi = mrb_bint_new_uint64(mrb, hi);
    mrb_value v_hi_shift = mrb_bint_lshift(mrb, v_hi, 64);
    return mrb_bint_add(mrb, v_hi_shift, v_lo);
  }

  static inline mrb_value mrb_bint_new_int128(mrb_state* mrb, __int128 s) {
    bool neg = s < 0;
    unsigned __int128 mag = neg ? static_cast<unsigned __int128>(-s)
                                : static_cast<unsigned __int128>(s);
    mrb_value v = mrb_bint_new_uint128(mrb, mag);
    return neg ? mrb_bint_neg(mrb, v) : v;
  }
#endif
}

template <typename T>
MRB_API mrb_value mrb_convert_number(mrb_state* mrb, T value) {
  using namespace mrbcpp::number_converter;

  // --- Enums: recurse on underlying type
  if constexpr (std::is_enum_v<T>) {
    using U = std::underlying_type_t<T>;
    return mrb_convert_number(mrb, static_cast<U>(value));
  }

  // --- Floating point
  else if constexpr (std::is_floating_point_v<T>) {
#ifndef MRB_NO_FLOAT
    if constexpr (std::numeric_limits<T>::lowest() >= std::numeric_limits<mrb_float>::lowest() &&
                  std::numeric_limits<T>::max()    <= std::numeric_limits<mrb_float>::max()) {
      return mrb_float_value(mrb, static_cast<mrb_float>(value));
    } else {
      mrb_raise(mrb, E_RANGE_ERROR, "Float too large for mrb_float");
    }
#else
    mrb_raise(mrb, E_TYPE_ERROR, "Float support disabled");
#endif
  }

  // --- Integral types (standard widths)
  else if constexpr (std::is_integral_v<T>) {
    // Compile-time fast lanes
    if constexpr (type_fits_fixnum<T>()) {
      return mrb_fixnum_value(static_cast<mrb_int>(value));
    } else if constexpr (type_fits_int<T>()) {
      // mrb_int_value decides fixnum vs boxed internally
      return mrb_int_value(mrb, static_cast<mrb_int>(value));
    }

    // Runtime refinement for types that don't fit mrb_int entirely
    if constexpr (std::is_signed_v<T>) {
      // Signed: check full int-range first, then BigInt
      if (value >= MRB_INT_MIN && value <= MRB_INT_MAX) {
        return mrb_int_value(mrb, static_cast<mrb_int>(value));
      }
#ifdef MRB_USE_BIGINT
# ifdef MRB_INT64
      return mrb_bint_new_int(mrb, static_cast<mrb_int>(value));
# else
      return mrb_bint_new_int64(mrb, static_cast<int64_t>(value));
# endif
#else
      mrb_raise(mrb, E_RANGE_ERROR, "Signed integer too large for mrb_int and BigInt disabled");
#endif
    } else {
      // Unsigned: only upper bound matters; lower bound is always >= 0
      if (value <= static_cast<std::make_unsigned_t<mrb_int>>(MRB_INT_MAX)) {
        return mrb_int_value(mrb, static_cast<mrb_int>(value));
      }
#ifdef MRB_USE_BIGINT
      return mrb_bint_new_uint64(mrb, static_cast<uint64_t>(value));
#else
      mrb_raise(mrb, E_RANGE_ERROR, "Unsigned integer too large for mrb_int and BigInt disabled");
#endif
    }
  }

  // --- Explicit 128-bit handling
#if defined(__SIZEOF_INT128__)
  else if constexpr (is_int128<T>::value) {
    // If representable by mrb_int, prefer that
    if (value >= static_cast<T>(MRB_INT_MIN) && value <= static_cast<T>(MRB_INT_MAX)) {
      return mrb_int_value(mrb, static_cast<mrb_int>(value));
    }
#ifdef MRB_USE_BIGINT
    return mrb_bint_new_int128(mrb, static_cast<__int128>(value));
#else
    mrb_raise(mrb, E_RANGE_ERROR, "__int128 too large and BigInt disabled");
    return mrb_undef_value();
#endif
  }
  else if constexpr (is_uint128<T>::value) {
    if (value <= static_cast<T>(MRB_INT_MAX)) {
      return mrb_int_value(mrb, static_cast<mrb_int>(value));
    }
#ifdef MRB_USE_BIGINT
    return mrb_bint_new_uint128(mrb, static_cast<unsigned __int128>(value));
#else
    mrb_raise(mrb, E_RANGE_ERROR, "unsigned __int128 too large and BigInt disabled");
#endif
  }
#endif

  // --- Unsupported numeric type fallback
  mrb_raise(mrb, E_TYPE_ERROR, "Unsupported numeric type");
}
