#include <mruby.h>
#include <mruby/num_helpers.h>
#include <mruby/num_helpers.hpp>
#include <mruby/value.h>

MRB_API mrb_value mrb_convert_int8(mrb_state* mrb, int8_t value) { return mrb_convert_number(mrb, value); }
MRB_API mrb_value mrb_convert_uint8(mrb_state* mrb, uint8_t value) { return mrb_convert_number(mrb, value); }

#if MRB_INT_BIT >= 16
MRB_API mrb_value mrb_convert_int16(mrb_state* mrb, int16_t value) { return mrb_convert_number(mrb, value); }
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 32
MRB_API mrb_value mrb_convert_uint16(mrb_state* mrb, uint16_t value) { return mrb_convert_number(mrb, value); }
#endif
#endif

#if MRB_INT_BIT >= 32
MRB_API mrb_value mrb_convert_int32(mrb_state* mrb, int32_t value) { return mrb_convert_number(mrb, value); }
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 64
MRB_API mrb_value mrb_convert_uint32(mrb_state* mrb, uint32_t value) { return mrb_convert_number(mrb, value); }
#endif
#endif

#if MRB_INT_BIT >= 64
MRB_API mrb_value mrb_convert_int64(mrb_state* mrb, int64_t value) { return mrb_convert_number(mrb, value); }
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 128
MRB_API mrb_value mrb_convert_uint64(mrb_state* mrb, uint64_t value) { return mrb_convert_number(mrb, value); }
#endif
#endif

#ifndef MRB_NO_FLOAT
MRB_API mrb_value mrb_convert_float(mrb_state* mrb, float value) { return mrb_convert_number(mrb, value); }
#ifndef MRB_USE_FLOAT32
MRB_API mrb_value mrb_convert_double(mrb_state* mrb, double value) { return mrb_convert_number(mrb, value); }
#endif
#endif
