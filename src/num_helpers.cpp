#include <mruby.h>
#include <mruby/num_helpers.h>
#include <mruby/num_helpers.hpp>
#include <mruby/value.h>

#define MRB_DEFINE_CONVERTER(Type, Name) \
  MRB_API mrb_value mrb_convert_##Name(mrb_state* mrb, Type value) { \
    return mrb_convert_number(mrb, value); \
  }

MRB_DEFINE_CONVERTER(int8_t, int8)
MRB_DEFINE_CONVERTER(uint8_t, uint8)

#if MRB_INT_BIT >= 16
MRB_DEFINE_CONVERTER(int16_t, int16)
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 32
MRB_DEFINE_CONVERTER(uint16_t, uint16)
#endif
#endif

#if MRB_INT_BIT >= 32
MRB_DEFINE_CONVERTER(int32_t, int32)
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 64
MRB_DEFINE_CONVERTER(uint32_t, uint32)
#endif
#endif

#if MRB_INT_BIT >= 64
MRB_DEFINE_CONVERTER(int64_t, int64)
#if defined(MRB_USE_BIGINT) || MRB_INT_BIT >= 128
MRB_DEFINE_CONVERTER(uint64_t, uint64)
#endif
#endif

#ifndef MRB_NO_FLOAT
MRB_DEFINE_CONVERTER(float, float)
#ifndef MRB_USE_FLOAT32
MRB_DEFINE_CONVERTER(double, double)
#endif
#endif

MRB_DEFINE_CONVERTER(short, short)
MRB_DEFINE_CONVERTER(unsigned short, ushort)
MRB_DEFINE_CONVERTER(int, int)
MRB_DEFINE_CONVERTER(unsigned int, uint)
MRB_DEFINE_CONVERTER(long, long)
MRB_DEFINE_CONVERTER(unsigned long, ulong)
MRB_DEFINE_CONVERTER(long long, long_long)
MRB_DEFINE_CONVERTER(unsigned long long, ulong_long)