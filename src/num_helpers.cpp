#include <mruby.h>
#include <mruby/num_helpers.h>
#include <mruby/num_helpers.hpp>
#include <mruby/value.h>
#include <limits>
#include <type_traits>

namespace mrbcpp::number_converter {
  template <typename T>
  constexpr static bool fits_in_mrb_int() {
#if defined(MRB_USE_BIGINT)
    return true;
#else
    using mrb_int_limits = std::numeric_limits<mrb_int>;
    using T_limits = std::numeric_limits<T>;

    if constexpr (std::is_signed<T>::value) {
      return T_limits::min() >= mrb_int_limits::min &&
            T_limits::max() <= mrb_int_limits::max;
    } else {
      // unsigned types must not exceed mrb_int's max
      return T_limits::max() <= mrb_int_limits::max;
    }
#endif
  }
  template <typename T>
  static mrb_value mrb_convert_number_safe(mrb_state* mrb, T value) {
    static_assert(std::is_integral<T>::value, "Expected integral type");
    if constexpr (fits_in_mrb_int<T>()) {
      return mrb_convert_number(mrb, value);
    } else {
      mrb_raise(mrb, E_RANGE_ERROR, "Type too large for current mruby build");
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