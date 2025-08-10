#pragma once
#include "branch_pred.h"
#include <mruby.h>
#include <mruby/string.h>
#include <string.h>
#include <mruby/value.h>
#include <mruby/numeric.h>
#include <mruby/dump.h>
#include <sys/types.h>

MRB_BEGIN_DECL

MRB_API mrb_value mrb_convert_int8(mrb_state* mrb, int8_t value);
MRB_API mrb_value mrb_convert_uint8(mrb_state* mrb, uint8_t value);

MRB_API mrb_value mrb_convert_int16(mrb_state* mrb, int16_t value);
MRB_API mrb_value mrb_convert_uint16(mrb_state* mrb, uint16_t value);

MRB_API mrb_value mrb_convert_int32(mrb_state* mrb, int32_t value);
MRB_API mrb_value mrb_convert_uint32(mrb_state* mrb, uint32_t value);

MRB_API mrb_value mrb_convert_int64(mrb_state* mrb, int64_t value);
MRB_API mrb_value mrb_convert_uint64(mrb_state* mrb, uint64_t value);

#ifndef MRB_NO_FLOAT
MRB_API mrb_value mrb_convert_float(mrb_state* mrb, float value);
#ifndef MRB_USE_FLOAT32
MRB_API mrb_value mrb_convert_double(mrb_state* mrb, double value);
#endif
#endif

MRB_API mrb_value mrb_convert_short(mrb_state* mrb, short value);
MRB_API mrb_value mrb_convert_int(mrb_state* mrb, int value);
MRB_API mrb_value mrb_convert_long(mrb_state* mrb, long value);
MRB_API mrb_value mrb_convert_long_long(mrb_state* mrb, long long value);
MRB_API mrb_value mrb_convert_ushort(mrb_state* mrb, unsigned short value);
MRB_API mrb_value mrb_convert_uint(mrb_state* mrb, unsigned int value);
MRB_API mrb_value mrb_convert_ulong(mrb_state* mrb, unsigned long value);
MRB_API mrb_value mrb_convert_ulong_long(mrb_state* mrb, unsigned long long value);

MRB_API mrb_value mrb_convert_size_t(mrb_state* mrb, size_t value);
MRB_API mrb_value mrb_convert_ssize_t(mrb_state* mrb, ssize_t value);


MRB_API mrb_value MRB_ENCODE_FIX_NAT(mrb_state *mrb, mrb_int numeric);
MRB_API mrb_value MRB_DECODE_FIX_NAT(mrb_state *mrb, mrb_value bin);
MRB_API mrb_value MRB_ENCODE_FIX_LE(mrb_state *mrb, mrb_int numeric);
MRB_API mrb_value MRB_DECODE_FIX_LE(mrb_state *mrb, mrb_value bin);
MRB_API mrb_value MRB_ENCODE_FIX_BE(mrb_state *mrb, mrb_int numeric);
MRB_API mrb_value MRB_DECODE_FIX_BE(mrb_state *mrb, mrb_value bin);

#ifndef MRB_NO_FLOAT
MRB_API mrb_value MRB_ENCODE_FLO_NAT(mrb_state *mrb, mrb_float numeric);
MRB_API mrb_value MRB_DECODE_FLO_NAT(mrb_state *mrb, mrb_value bin);
MRB_API mrb_value MRB_ENCODE_FLO_LE(mrb_state *mrb, mrb_float numeric);
MRB_API mrb_value MRB_DECODE_FLO_LE(mrb_state *mrb, mrb_value bin);
MRB_API mrb_value MRB_ENCODE_FLO_BE(mrb_state *mrb, mrb_float numeric);
MRB_API mrb_value MRB_DECODE_FLO_BE(mrb_state *mrb, mrb_value bin);

#define MRB_POSNUMB2NUM(mrb, number) ((POSFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#define MRB_NEGNUMB2NUM(mrb, number) ((NEGFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#define MRB_NUMB2NUM(mrb, number) ((FIXABLE(number)) ? mrb_fixnum_value(number) : mrb_float_value(mrb, number))

#else

#define MRB_POSNUMB2NUM(mrb, number) ((POSFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#define MRB_NEGNUMB2NUM(mrb, number) ((NEGFIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#define MRB_NUMB2NUM(mrb, number) ((FIXABLE(number)) ? mrb_fixnum_value(number) : mrb_raise(mrb, E_RANGE_ERROR, "Number doesn't fit into Numeric"))

#endif


MRB_END_DECL