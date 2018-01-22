#ifndef NUM_HELPERS_H
#define NUM_HELPERS_H

#include "branch_pred.h"
#include <mruby.h>
#include <mruby/string.h>
#include <string.h>
#include <mruby/value.h>
#include <mruby/numeric.h>
#include <mruby/dump.h>

MRB_BEGIN_DECL

MRB_INLINE mrb_value
MRB_ENCODE_FIX_NAT(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));
  memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FIX_NAT(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  return mrb_fixnum_value(numeric);
}

MRB_INLINE mrb_value
MRB_ENCODE_FIX_LE(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

  if (bigendian_p()) {
    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    for (int i = 0; i < sizeof(numeric) - 1; i++) {
      dst[i] = (uint8_t) numeric;
      numeric >>= 8;
    }
    dst[sizeof(numeric) - 1] = (uint8_t) numeric;
  } else {
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  }

  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FIX_LE(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  if (bigendian_p()) {
    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    numeric = (mrb_int) src[0];
    for (int i = 1; i < sizeof(numeric); i++) {
      numeric |= (mrb_int) src[i] << (8 * i);
    }
  } else {
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  }

  return mrb_fixnum_value(numeric);
}

MRB_INLINE mrb_value
MRB_ENCODE_FIX_BE(mrb_state *mrb, mrb_int numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

  if (bigendian_p()) {
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  } else {
    uint8_t *dst = (uint8_t *) RSTRING_PTR(bin);
    for (int i = sizeof(numeric) -1;i > 0; i--) {
      dst[i] = (uint8_t) numeric;
      numeric >>= 8;
    }
    dst[0] = (uint8_t) numeric;
  }

  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FIX_BE(mrb_state *mrb, mrb_value bin)
{
  mrb_int numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  if (bigendian_p()) {
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  } else {
    const uint8_t *src = (const uint8_t *) RSTRING_PTR(bin);
    numeric = (mrb_int) src[sizeof(numeric) - 1];
    for (int i = sizeof(numeric) - 2; i >= 0; i--) {
      numeric |= (mrb_int) src[i] << (sizeof(numeric) * (8 - i - 1));
    }
  }

  return mrb_fixnum_value(numeric);
}

#ifndef MRB_WITHOUT_FLOAT
MRB_INLINE mrb_value
MRB_ENCODE_FLO_NAT(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));
  memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FLO_NAT(mrb_state *mrb, mrb_value bin)
{
  mrb_float numeric;

  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
  return mrb_float_value(mrb, numeric);
}

MRB_INLINE mrb_value
MRB_ENCODE_FLO_LE(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

  if (bigendian_p()) {
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
  } else {
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  }

  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FLO_LE(mrb_state *mrb, mrb_value bin)
{
  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(mrb_float)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  if (bigendian_p()) {
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
  } else {
    mrb_float numeric;
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
    return mrb_float_value(mrb, numeric);
  }
}

MRB_INLINE mrb_value
MRB_ENCODE_FLO_BE(mrb_state *mrb, mrb_float numeric)
{
  mrb_value bin = mrb_str_new(mrb, NULL, sizeof(numeric));

  if (bigendian_p()) {
    memcpy((uint8_t *) RSTRING_PTR(bin), &numeric, sizeof(numeric));
  } else {
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
  }

  return bin;
}

MRB_INLINE mrb_value
MRB_DECODE_FLO_BE(mrb_state *mrb, mrb_value bin)
{
  if (unlikely(!mrb_string_p(bin))) mrb_raise(mrb, E_TYPE_ERROR, "Not a String");
  if (RSTRING_LEN(bin) != sizeof(mrb_float)) mrb_raise(mrb, E_ARGUMENT_ERROR, "Encoded Data cannot be decoded");

  if (bigendian_p()) {
    mrb_float numeric;
    memcpy(&numeric, (const uint8_t *) RSTRING_PTR(bin), sizeof(numeric));
    return mrb_float_value(mrb, numeric);
  } else {
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
  }
}

#define MRB_POSNUMB2NUM(mrb, number) ((POSFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#define MRB_NEGNUMB2NUM(mrb, number) ((NEGFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#define MRB_NUMB2NUM(mrb, number) ((FIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#else // ifndef MRB_WITHOUT_FLOAT

#define MRB_POSNUMB2NUM(mrb, number) ((POSFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#define MRB_NEGNUMB2NUM(mrb, number) ((NEGFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#define MRB_NUMB2NUM(mrb, number) ((FIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#endif

MRB_END_DECL

#endif
