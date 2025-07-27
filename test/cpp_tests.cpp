#include <mruby.h>
#include <mruby/cpp_helpers.hpp>
#include <mruby/num_helpers.hpp>
#include <mruby/num_helpers.h>
#include <mruby/value.h>


MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb)
{
  mrb_convert_number(mrb, 42);
  mrb_convert_int8(mrb, static_cast<int8_t>(42));

  mrb_convert_number(mrb, static_cast<int8_t>(42));
  mrb_convert_number(mrb, static_cast<uint8_t>(42));

#if MRB_INT_BIT >= 16
  mrb_convert_number(mrb, static_cast<int16_t>(32767));
  mrb_convert_number(mrb, static_cast<int16_t>(-32768));
  mrb_convert_int16(mrb, static_cast<int16_t>(32767));
  mrb_convert_int16(mrb, static_cast<int16_t>(-32768));
#endif

#if MRB_INT_BIT >= 32
  mrb_convert_number(mrb, static_cast<uint16_t>(65535));
  mrb_convert_number(mrb, static_cast<int32_t>(2147483647));
  mrb_convert_number(mrb, static_cast<int32_t>(-2147483647));
  mrb_convert_number(mrb, static_cast<int32_t>(-2147483648LL));
  mrb_convert_uint16(mrb, static_cast<uint16_t>(65535));
  mrb_convert_int32(mrb, static_cast<int32_t>(2147483647));
  mrb_convert_int32(mrb, static_cast<int32_t>(-2147483647));
  mrb_convert_int32(mrb, static_cast<int32_t>(-2147483648LL));
#endif

#if MRB_INT_BIT >= 64
  mrb_convert_number(mrb, static_cast<uint32_t>(4294967295U));
  mrb_convert_number(mrb, static_cast<int64_t>(9223372036854775807LL));
  mrb_convert_number(mrb, static_cast<int64_t>(-9223372036854775807LL - 1LL));
  mrb_convert_uint32(mrb, static_cast<uint32_t>(4294967295U));
  mrb_convert_int64(mrb, static_cast<int64_t>(9223372036854775807LL));
  mrb_convert_int64(mrb, static_cast<int64_t>(-9223372036854775807LL - 1LL));
#endif

#ifndef MRB_WITHOUT_FLOAT
  // Always test float
    mrb_convert_number(mrb, static_cast<float>(3.14f));
    mrb_convert_number(mrb, static_cast<float>(3.402823466e38f));
    mrb_convert_float(mrb, static_cast<float>(3.14f));
    mrb_convert_float(mrb, static_cast<float>(3.402823466e38f));
#ifndef MRB_USE_FLOAT32
    mrb_convert_number(mrb, static_cast<double>(3.14));
    mrb_convert_number(mrb, static_cast<double>(1.7976931348623157e308));
    mrb_convert_double(mrb, static_cast<double>(3.14));
    mrb_convert_double(mrb, static_cast<double>(1.7976931348623157e308));
#endif
#endif

#ifdef MRB_USE_BIGINT
  // Test unsigned that must use BigInt
  #if MRB_INT_BIT == 16
    constexpr uint64_t big_uint = 70000ULL; // > 65535
  #elif MRB_INT_BIT == 32
    constexpr uint64_t big_uint = 5000000000ULL; // > 4294967295
  #elif MRB_INT_BIT == 64
    constexpr uint64_t big_uint = 18446744073709551615ULL; // max uint64_t
  #else
    mrb_raise(mrb, E_RUNTIME_ERROR, "Unsupported MRB_INT_BIT size for BigInt test");
  #endif

  mrb_convert_number(mrb, big_uint);

  // Test signed large positive that must use BigInt
  #if MRB_INT_BIT == 16
    constexpr int64_t big_int = 70000;
  #elif MRB_INT_BIT == 32
    constexpr int64_t big_int = 5000000000LL;
  #elif MRB_INT_BIT == 64
    constexpr int64_t big_int = 9223372036854775807LL; // This may still fit if mrb_int is int64_t
  #endif

  mrb_convert_number(mrb, big_int);

  // Also test negative BigInt
  #if MRB_INT_BIT == 16
    constexpr int64_t big_neg_int = -70000;
  #elif MRB_INT_BIT == 32
    constexpr int64_t big_neg_int = -5000000000LL;
  #elif MRB_INT_BIT == 64
    constexpr int64_t big_neg_int = -9223372036854775807LL - 1LL;
  #endif

  mrb_convert_number(mrb, big_neg_int);
#endif

}
MRB_END_DECL
