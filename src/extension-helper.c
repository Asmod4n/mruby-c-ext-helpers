#include <mruby/num_helpers.h>
#include <mruby/string.h>

static mrb_value
mrb_str_incr(mrb_state *mrb, mrb_value self)
{
  mrb_int numeric;
  if (RSTRING_LEN(self) != sizeof(numeric)) mrb_raise(mrb, E_ARGUMENT_ERROR, "supplied string has wrong size");
  mrb_str_modify(mrb, RSTRING(self));

  memcpy(&numeric, (const uint8_t *) RSTRING_PTR(self), sizeof(numeric));
  ++numeric;
  memcpy((uint8_t *) RSTRING_PTR(self), &numeric, sizeof(numeric));

  return self;
}

static mrb_value
mrb_bin2fix(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FIX_NAT(mrb, self);
}

static mrb_value
mrb_fix2bin(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FIX_NAT(mrb, mrb_fixnum(self));
}

static mrb_value
mrb_bin2fix_le(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FIX_LE(mrb, self);
}

static mrb_value
mrb_fix2bin_le(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FIX_LE(mrb, mrb_fixnum(self));
}

static mrb_value
mrb_bin2fix_be(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FIX_BE(mrb, self);
}

static mrb_value
mrb_fix2bin_be(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FIX_BE(mrb, mrb_fixnum(self));
}

#ifndef MRB_WITHOUT_FLOAT
static mrb_value
mrb_flo2bin(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FLO_NAT(mrb, mrb_float(self));
}

static mrb_value
mrb_bin2flo(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FLO_NAT(mrb, self);
}

static mrb_value
mrb_flo2bin_le(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FLO_LE(mrb, mrb_float(self));
}

static mrb_value
mrb_bin2flo_le(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FLO_LE(mrb, self);
}

static mrb_value
mrb_flo2bin_be(mrb_state *mrb, mrb_value self)
{
  return MRB_ENCODE_FLO_BE(mrb, mrb_float(self));
}

static mrb_value
mrb_bin2flo_be(mrb_state *mrb, mrb_value self)
{
  return MRB_DECODE_FLO_BE(mrb, self);
}
#endif // MRB_WITHOUT_FLOAT

void
mrb_mruby_c_ext_helpers_gem_init(mrb_state* mrb)
{
  mrb_define_method(mrb, mrb->string_class, "incr", mrb_str_incr, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->string_class, "to_fix", mrb_bin2fix, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->fixnum_class, "to_bin", mrb_fix2bin, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->string_class, "to_fix_le", mrb_bin2fix_le, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->fixnum_class, "to_bin_le", mrb_fix2bin_le, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->string_class, "to_fix_be", mrb_bin2fix_be, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->fixnum_class, "to_bin_be", mrb_fix2bin_be, MRB_ARGS_NONE());
#ifndef MRB_WITHOUT_FLOAT
  mrb_define_method(mrb, mrb->string_class, "to_flo", mrb_bin2flo, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->float_class,  "to_bin", mrb_flo2bin, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->string_class, "to_flo_le", mrb_bin2flo_le, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->float_class,  "to_bin_le", mrb_flo2bin_le, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->string_class, "to_flo_be", mrb_bin2flo_be, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->float_class,  "to_bin_be", mrb_flo2bin_be, MRB_ARGS_NONE());
#endif
}

void mrb_mruby_c_ext_helpers_gem_final(mrb_state* mrb) {}
