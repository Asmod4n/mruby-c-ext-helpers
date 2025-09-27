#include <mruby.h>
#include <mruby/num_helpers.h>
#include <mruby/num_helpers.hpp>
#include <mruby/value.h>
#include <limits>
#include <type_traits>
#include <string>

namespace mrbcpp::number_converter {
  template <typename T>
  constexpr bool fits_in_mrb_int() {
#ifdef MRB_USE_BIGINT
    return true;
#else
    using mrb_int_limits = std::numeric_limits<mrb_int>;
    using T_limits       = std::numeric_limits<T>;

#if defined(__SIZEOF_INT128__)
    if constexpr (std::is_same_v<T, __int128>) {
      // Signed 128: must be within mrb_int range
      return (static_cast<__int128>(mrb_int_limits::min()) >= T_limits::min()) &&
             (static_cast<__int128>(mrb_int_limits::max()) <= T_limits::max());
    }
    if constexpr (std::is_same_v<T, unsigned __int128>) {
      // Unsigned 128: only upper bound matters
      return static_cast<unsigned __int128>(mrb_int_limits::max()) >= T_limits::max();
    }
#endif

    if constexpr (std::is_signed_v<T>) {
      return T_limits::min() >= mrb_int_limits::min() &&
             T_limits::max() <= mrb_int_limits::max();
    } else if constexpr (std::is_unsigned_v<T>) {
      return T_limits::max() <= static_cast<std::make_unsigned_t<mrb_int>>(mrb_int_limits::max());
    } else {
      return false;
    }
  #endif
  }

  template <typename T>
  std::string type_name_from_signature() {
  #if defined(__clang__) || defined(__GNUC__)
    std::string sig = __PRETTY_FUNCTION__;
    auto start = sig.find("T = ") + 4;
    auto end = sig.find(']', start);
    return sig.substr(start, end - start);
  #elif defined(_MSC_VER)
    std::string sig = __FUNCSIG__;
    auto start = sig.find("T = ") + 4;
    auto end = sig.find('>');
    return sig.substr(start, end - start);
  #else
    return "unknown";
  #endif
  }

  template <typename T>
  struct is_extended_integral : std::is_integral<T> {};

  #if defined(__SIZEOF_INT128__)
  template <> struct is_extended_integral<__int128> : std::true_type {};
  template <> struct is_extended_integral<unsigned __int128> : std::true_type {};
  #endif

  template <typename T>
  inline constexpr bool is_extended_integral_v = is_extended_integral<T>::value;


  template <typename T>
  static mrb_value mrb_convert_number_safe(mrb_state* mrb, T value) {
    static_assert(is_extended_integral_v<T>, "Expected integral type");
    if constexpr (fits_in_mrb_int<T>()) {
      return mrb_convert_number(mrb, value);
    } else {
      mrb_raise(mrb, E_RANGE_ERROR,
        ("mruby was compiled with MRB_INT_BIT = " + std::to_string(MRB_INT_BIT) +
        ", but attempted to convert type '" + type_name_from_signature<T>() +
        "' (size = " + std::to_string(sizeof(T) * 8) + " bits), which exceeds the supported range.").c_str());
    }
  }
}

#define MRB_DEFINE_CONVERTER(Type, Name) \
  MRB_API mrb_value mrb_convert_##Name(mrb_state* mrb, Type value) { \
    return mrbcpp::number_converter::mrb_convert_number_safe(mrb, value); \
  }

MRB_DEFINE_CONVERTER(int8_t, int8)
MRB_DEFINE_CONVERTER(uint8_t, uint8)
MRB_DEFINE_CONVERTER(int16_t, int16)
MRB_DEFINE_CONVERTER(uint16_t, uint16)
MRB_DEFINE_CONVERTER(int32_t, int32)
MRB_DEFINE_CONVERTER(uint32_t, uint32)
MRB_DEFINE_CONVERTER(int64_t, int64)
MRB_DEFINE_CONVERTER(uint64_t, uint64)
#if defined(__SIZEOF_INT128__)
MRB_DEFINE_CONVERTER(__int128, int128)
MRB_DEFINE_CONVERTER(unsigned __int128, uint128)
#endif
MRB_DEFINE_CONVERTER(short, short)
MRB_DEFINE_CONVERTER(unsigned short, ushort)
MRB_DEFINE_CONVERTER(int, int)
MRB_DEFINE_CONVERTER(unsigned int, uint)
MRB_DEFINE_CONVERTER(long, long)
MRB_DEFINE_CONVERTER(unsigned long, ulong)
MRB_DEFINE_CONVERTER(long long, long_long)
MRB_DEFINE_CONVERTER(unsigned long long, ulong_long)
MRB_DEFINE_CONVERTER(size_t, size_t)
MRB_DEFINE_CONVERTER(ssize_t, ssize_t)


#ifndef MRB_NO_FLOAT
MRB_API mrb_value mrb_convert_float(mrb_state* mrb, float value) { \
  return mrb_convert_number(mrb, value); \
}
#ifndef MRB_USE_FLOAT32
MRB_API mrb_value mrb_convert_double(mrb_state* mrb, double value) { \
  return mrb_convert_number(mrb, value); \
}
#endif
#endif

MRB_API mrb_value
MRB_ENCODE_FIX_NAT(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));
  memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  return bin;
}

MRB_API mrb_value
MRB_DECODE_FIX_NAT(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  return mrb_fixnum_value(numeric);
}

MRB_API mrb_value
MRB_ENCODE_FIX_LE(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

#ifdef MRB_ENDIAN_BIG
    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    for (int i = 0; i < sizeof(numeric) - 1; i++) {
      dst[i] = (uint8_t) numeric;
      numeric >>= 8;
    }
    dst[sizeof(numeric) - 1] = (uint8_t) numeric;
#else
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
#endif

  return bin;
}

MRB_API mrb_value
MRB_DECODE_FIX_LE(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

#ifdef MRB_ENDIAN_BIG
    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    numeric = (mrb_int) src[0];
    for (int i = 1; i < sizeof(numeric); i++) {
      numeric |= (mrb_int) src[i] << (8 * i);
    }
#else
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
#endif

  return mrb_fixnum_value(numeric);
}

MRB_API mrb_value
MRB_ENCODE_FIX_BE(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

#ifdef MRB_ENDIAN_BIG
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
#else
    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    for (int i = sizeof(numeric) -1;i > 0; i--) {
      dst[i] = (uint8_t) numeric;
      numeric >>= 8;
    }
    dst[0] = (uint8_t) numeric;
#endif

  return bin;
}

MRB_API mrb_value
MRB_DECODE_FIX_BE(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

#ifdef MRB_ENDIAN_BIG
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
#else
    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    numeric = (mrb_int) src[sizeof(numeric) - 1];
    for (int i = sizeof(numeric) - 2; i >= 0; i--) {
      numeric |= (mrb_int) src[i] << (sizeof(numeric) * (8 - i - 1));
    }
#endif

  return mrb_fixnum_value(numeric);
}

#ifndef MRB_NO_FLOAT
MRB_API mrb_value
MRB_ENCODE_FLO_NAT(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));
  memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  return bin;
}

MRB_API mrb_value
MRB_DECODE_FLO_NAT(mrb_state *mrb, mrb_value bin)
{
  mrb_float numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  return mrb_float_value(mrb, numeric);
}

MRB_API mrb_value
MRB_ENCODE_FLO_LE(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

#ifdef MRB_ENDIAN_BIG
    if (unlikely(sizeof(mrb_float) != sizeof(mrb_int))) mrb_raise(mrb, E_RUNTIME_ERROR, "size of mrb_float and mrb_int differ, cannot encode floats.");

    union {
      mrb_int i;
      mrb_float f;
    } swap;

    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    swap.f = numeric;
    for (int i = 0; i < sizeof(swap) - 1; i++) {
      dst[i] = (uint8_t) swap.i;
      swap.i >>= 8;
    }
    dst[sizeof(swap) - 1] = (uint8_t) swap.i;
#else
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
#endif

  return bin;
}

MRB_API mrb_value
MRB_DECODE_FLO_LE(mrb_state *mrb, mrb_value bin)
{
  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(mrb_float)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

#ifdef MRB_ENDIAN_BIG
    if (unlikely(sizeof(mrb_float) != sizeof(mrb_int))) mrb_raise(mrb, E_RUNTIME_ERROR, "size of mrb_float and mrb_int differ, cannot decode floats.");

    union {
      mrb_int i;
      mrb_float f;
    } swap;

    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    swap.i = (mrb_int) src[0];
    for (int i = 1; i < sizeof(swap); i++) {
      swap.i |= (mrb_int) src[i] << (8 * i);
    }

    return mrb_float_value(mrb, swap.f);
#else
    mrb_float numeric;
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
    return mrb_float_value(mrb, numeric);
#endif
}

MRB_API mrb_value
MRB_ENCODE_FLO_BE(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

#ifdef MRB_ENDIAN_BIG
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
#else
    if (unlikely(sizeof(mrb_float) != sizeof(mrb_int))) mrb_raise(mrb, E_RUNTIME_ERROR, "size of mrb_float and mrb_int differ, cannot encode floats.");

    union {
      mrb_int i;
      mrb_float f;
    } swap;

    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    swap.f = numeric;
    for (int i = sizeof(swap) -1;i > 0; i--) {
      dst[i] = (uint8_t) swap.i;
      swap.i >>= 8;
    }
    dst[0] = (uint8_t) swap.i;
#endif

  return bin;
}

MRB_API mrb_value
MRB_DECODE_FLO_BE(mrb_state *mrb, mrb_value bin)
{
  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(mrb_float)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

#ifdef MRB_ENDIAN_BIG
    mrb_float numeric;
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
    return mrb_float_value(mrb, numeric);
#else
    if (unlikely(sizeof(mrb_float) != sizeof(mrb_int))) mrb_raise(mrb, E_RUNTIME_ERROR, "size of mrb_float and mrb_int differ, cannot decode floats.");

    union {
      mrb_int i;
      mrb_float f;
    } swap;

    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    swap.i = (mrb_int) src[sizeof(swap) - 1];
    for (int i = sizeof(swap) - 2; i >= 0; i--) {
      swap.i |= (mrb_int) src[i] << (sizeof(swap) * (8 - i - 1));
    }

    return mrb_float_value(mrb, swap.f);
#endif
}
#endif