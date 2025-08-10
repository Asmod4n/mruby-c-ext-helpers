#include <mruby.h>
#include <mruby/value.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/presym.h>
#include <mruby/string.h>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <mruby/num_helpers.hpp>
#include <mruby/mrb_convert_cpp_value.hpp>

static void run_tests(mrb_state *mrb)
{
// ✅ Basic numeric
  mrb_value i = mrb_convert_cpp_value(mrb, 42);
  assert(mrb_type(i) == MRB_TT_INTEGER);
  assert(mrb_integer(i) == 42);

  mrb_value f = mrb_convert_cpp_value(mrb, 3.14);
  assert(mrb_type(f) == MRB_TT_FLOAT);
constexpr double epsilon = 1e-6;
assert(std::abs(mrb_float(f) - 3.14) < epsilon); // ✅ safe


  // ✅ String
  mrb_value s = mrb_convert_cpp_value(mrb, std::string("hello"));
  assert(mrb_type(s) == MRB_TT_STRING);
  assert(std::string(RSTRING_PTR(s), RSTRING_LEN(s)) == "hello");

  // ✅ Boolean
  mrb_value b1 = mrb_convert_cpp_value(mrb, true);
  mrb_value b2 = mrb_convert_cpp_value(mrb, false);
assert(mrb_bool(b1) == true);
assert(mrb_bool(b2) == false);


  // ✅ Nil
  mrb_value n = mrb_convert_cpp_value(mrb, nullptr);
  assert(mrb_type(n) == MRB_TT_FALSE);

  // ✅ Vector → Array
  std::vector<int> v = {1, 2, 3};
  mrb_value arr = mrb_convert_cpp_value(mrb, v);
  assert(mrb_type(arr) == MRB_TT_ARRAY);
  assert(RARRAY_LEN(arr) == 3);
  assert(mrb_integer(mrb_ary_ref(mrb, arr, 0)) == 1);

  // ✅ Map → Hash
  std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
  mrb_value h = mrb_convert_cpp_value(mrb, m);
  assert(mrb_type(h) == MRB_TT_HASH);
  mrb_value key_a = mrb_str_new_cstr(mrb, "a");
  mrb_value val_a = mrb_hash_get(mrb, h, key_a);
  assert(mrb_type(val_a) == MRB_TT_INTEGER);
  assert(mrb_integer(val_a) == 1);

  // ✅ system_clock → Time
  auto sys_now = std::chrono::system_clock::now();
  mrb_value sys_time = mrb_convert_cpp_value(mrb, sys_now);
  assert(mrb_obj_is_kind_of(mrb, sys_time, mrb_class_get_id(mrb, MRB_SYM(Time))));

  // ✅ steady_clock → Time (converted via system_clock)
  auto steady_now = std::chrono::steady_clock::now();
  mrb_value steady_time = mrb_convert_cpp_value(mrb, steady_now);
  assert(mrb_obj_is_kind_of(mrb, steady_time, mrb_class_get_id(mrb, MRB_SYM(Time))));

  // ✅ high_resolution_clock → Time (converted via system_clock)
  auto highres_now = std::chrono::high_resolution_clock::now();
  mrb_value highres_time = mrb_convert_cpp_value(mrb, highres_now);
  assert(mrb_obj_is_kind_of(mrb, highres_time, mrb_class_get_id(mrb, MRB_SYM(Time))));

  // ✅ Check microsecond precision is preserved (within bounds)
  auto precise = std::chrono::system_clock::now();
  mrb_value precise_time = mrb_convert_cpp_value(mrb, precise);
  mrb_value usec_val = mrb_funcall_id(mrb, precise_time, MRB_SYM(usec), 0);
  assert(mrb_type(usec_val) == MRB_TT_INTEGER);
  assert(mrb_integer(usec_val) >= 0);
  assert(mrb_integer(usec_val) < 1000000);


  // ✅ Set
  std::set<std::string> tags = {"gpu", "debug"};
  mrb_value ruby_set = mrb_convert_cpp_value(mrb, tags);
  assert(mrb_obj_is_kind_of(mrb, ruby_set, mrb_class_get_id(mrb, MRB_SYM(Set))));

  // ✅ Array of strings
  std::array<std::string, 2> names = {"alice", "bob"};
  mrb_value name_ary = mrb_convert_cpp_value(mrb, names);
  assert(mrb_type(name_ary) == MRB_TT_ARRAY);
  assert(RSTRING_LEN(mrb_ary_ref(mrb, name_ary, 1)) == 3);

  // ✅ Unordered map → Hash
  std::unordered_map<std::string_view, bool> flags = {
    {"debug", true},
    {"gpu", false}
  };
  mrb_value flag_hash = mrb_convert_cpp_value(mrb, flags);
  mrb_value debug_key = mrb_str_new_cstr(mrb, "debug");
  mrb_value debug_val = mrb_hash_get(mrb, flag_hash, debug_key);
  assert(mrb_type(debug_val) == MRB_TT_TRUE);

  // ✅ Unordered set → Set
  std::unordered_set<int> ids = {10, 20, 30};
  mrb_value id_set = mrb_convert_cpp_value(mrb, ids);
  assert(mrb_obj_is_kind_of(mrb, id_set, mrb_class_get_id(mrb, MRB_SYM(Set))));

  // ✅ String literal
  mrb_value lit = mrb_convert_cpp_value(mrb, "mruby");
  assert(mrb_type(lit) == MRB_TT_STRING);
  assert(RSTRING_LEN(lit) == 5);
  assert(strncmp(RSTRING_PTR(lit), "mruby", 5) == 0);
}

MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb)
{
  run_tests(mrb);
}
MRB_END_DECL
