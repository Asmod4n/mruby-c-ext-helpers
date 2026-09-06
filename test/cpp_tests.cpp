/*
 * Test-only Ruby surface over this gem's C++ conversion helpers,
 * compiled into mrbtest and nothing else (mruby builds test/* of a gem
 * only for its test binary). Values that used to be parsed from Ruby
 * source at runtime are now either literals in test/test.rb (compiled
 * ahead of time by the build's mrbc) or built directly through the
 * mruby C API below -- neither path touches the compiler, so this gem
 * needs no mruby-compiler test dependency to reach full coverage.
 *
 * Two directions get their own probes:
 *   CExtHelpersVectors.any_roundtrip(val)
 *     -> mrb_value_to_any(val), then converts the resulting std::any
 *        back to an mrb_value so test.rb can assert_equal against the
 *        original. Exercises mrb_value_to_any / mrb_array_to_vector /
 *        mrb_hash_to_map, including the MRB_TT_STRUCT and MRB_TT_SET
 *        branches.
 *   CExtHelpersVectors.to_mrb_*
 *     -> each wraps one cpp_to_mrb_value<T> instantiation with a fixed
 *        C++-side input and returns the mrb_value it produced.
 *
 * The remaining checks (numeric edge cases, the MRB_CPP_DEFINE_TYPE
 * subclassing contract, the mrb_cpp_new/mrb_cpp_get round trip) are
 * pure C++/C-API exercises with nothing Ruby-observable to assert on
 * beyond "it ran to completion" -- they keep their original cassert
 * bodies and are exposed as `?`-suffixed predicates so test.rb still
 * drives them and mrbtest still reports them by name.
 */
#include <mruby.h>
#include <mruby/value.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/presym.h>
#include <mruby/string.h>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <chrono>
#include <any>
#include <mruby/cpp_to_mrb_value.hpp>
#include <mruby/mrb_value_to_cpp.hpp>
#include <mruby/cpp_helpers.hpp>

// -------------------------------------------------------------
// std::any -> mrb_value, the mirror of mrb_value_to_any, so a Ruby
// value can be sent through the library's conversion and compared to
// itself on the other side without a runtime compiler in between.
// -------------------------------------------------------------

static mrb_value any_to_mrb(mrb_state* mrb, const std::any& a);

static mrb_value
map_key_to_mrb(mrb_state* mrb, const MapKey& k)
{
  if (auto p = std::get_if<mrb_int>(&k)) return mrb_int_value(mrb, *p);
#ifndef MRB_NO_FLOAT
  if (auto p = std::get_if<mrb_float>(&k)) return mrb_float_value(mrb, *p);
#endif
  const auto& s = std::get<std::string>(k);
  return mrb_str_new(mrb, s.data(), s.size());
}

static mrb_value
any_to_mrb(mrb_state* mrb, const std::any& a)
{
  if (!a.has_value()) return mrb_nil_value();
  if (a.type() == typeid(bool)) return mrb_bool_value(std::any_cast<bool>(a));
  if (a.type() == typeid(mrb_int)) return mrb_int_value(mrb, std::any_cast<mrb_int>(a));
#ifndef MRB_NO_FLOAT
  if (a.type() == typeid(mrb_float)) return mrb_float_value(mrb, std::any_cast<mrb_float>(a));
#endif
  if (a.type() == typeid(std::string)) {
    const auto& s = std::any_cast<const std::string&>(a);
    return mrb_str_new(mrb, s.data(), s.size());
  }
  if (a.type() == typeid(std::vector<std::any>)) {
    const auto& v = std::any_cast<const std::vector<std::any>&>(a);
    mrb_value ary = mrb_ary_new_capa(mrb, static_cast<mrb_int>(v.size()));
    mrb_gc_protect(mrb, ary);
    int arena_index = mrb_gc_arena_save(mrb);
    for (const auto& item : v) {
      mrb_ary_push(mrb, ary, any_to_mrb(mrb, item));
      mrb_gc_arena_restore(mrb, arena_index);
    }
    return ary;
  }
  if (a.type() == typeid(std::map<MapKey, std::any>)) {
    const auto& m = std::any_cast<const std::map<MapKey, std::any>&>(a);
    mrb_value h = mrb_hash_new(mrb);
    mrb_gc_protect(mrb, h);
    int arena_index = mrb_gc_arena_save(mrb);
    for (const auto& [k, v] : m) {
      mrb_hash_set(mrb, h, map_key_to_mrb(mrb, k), any_to_mrb(mrb, v));
      mrb_gc_arena_restore(mrb, arena_index);
    }
    return h;
  }
  mrb_raise(mrb, E_TYPE_ERROR, "any_to_mrb: unhandled std::any content");
}

static mrb_value
any_roundtrip(mrb_state* mrb, mrb_value self)
{
  mrb_value val;
  mrb_get_args(mrb, "o", &val);
  std::any a = mrb_value_to_any(mrb, val);
  return any_to_mrb(mrb, a);
}

// -------------------------------------------------------------
// cpp_to_mrb_value<T> probes: fixed C++-side input in, mrb_value out,
// left for test.rb to check.
// -------------------------------------------------------------

static mrb_value
to_mrb_bool(mrb_state* mrb, mrb_value self)
{
  mrb_bool b;
  mrb_get_args(mrb, "b", &b);
  return cpp_to_mrb_value(mrb, static_cast<bool>(b));
}

static mrb_value
to_mrb_int(mrb_state* mrb, mrb_value self)
{
  mrb_int i;
  mrb_get_args(mrb, "i", &i);
  return cpp_to_mrb_value(mrb, static_cast<int>(i));
}

static mrb_value
to_mrb_double(mrb_state* mrb, mrb_value self)
{
  mrb_float f;
  mrb_get_args(mrb, "f", &f);
  return cpp_to_mrb_value(mrb, static_cast<double>(f));
}

static mrb_value
to_mrb_std_string(mrb_state* mrb, mrb_value self)
{
  return cpp_to_mrb_value(mrb, std::string("foo"));
}

static mrb_value
to_mrb_string_view(mrb_state* mrb, mrb_value self)
{
  return cpp_to_mrb_value(mrb, std::string_view("bar"));
}

static mrb_value
to_mrb_cstr(mrb_state* mrb, mrb_value self)
{
  return cpp_to_mrb_value(mrb, "baz");
}

static mrb_value
to_mrb_nullptr(mrb_state* mrb, mrb_value self)
{
  return cpp_to_mrb_value(mrb, nullptr);
}

static mrb_value
to_mrb_map(mrb_state* mrb, mrb_value self)
{
  std::map<std::string, int> smap = {{"a", 1}, {"b", 2}};
  return cpp_to_mrb_value(mrb, smap);
}

static mrb_value
to_mrb_unordered_map(mrb_state* mrb, mrb_value self)
{
  std::unordered_map<std::string, bool> umap = {{"x", true}, {"y", false}};
  return cpp_to_mrb_value(mrb, umap);
}

static mrb_value
to_mrb_set(mrb_state* mrb, mrb_value self)
{
  std::set<int> sset = {10, 20};
  return cpp_to_mrb_value(mrb, sset);
}

static mrb_value
to_mrb_unordered_set(mrb_state* mrb, mrb_value self)
{
  std::unordered_set<std::string> uset = {"foo", "bar"};
  return cpp_to_mrb_value(mrb, uset);
}

static mrb_value
to_mrb_vector(mrb_state* mrb, mrb_value self)
{
  std::vector<int> v = {1, 2, 3};
  return cpp_to_mrb_value(mrb, v);
}

static mrb_value
to_mrb_array(mrb_state* mrb, mrb_value self)
{
  std::array<std::string, 2> arr = {"x", "y"};
  return cpp_to_mrb_value(mrb, arr);
}

static mrb_value
to_mrb_time(mrb_state* mrb, mrb_value self)
{
  return cpp_to_mrb_value(mrb, std::chrono::system_clock::now());
}

// ----------------------------------------------
// Subclassing tests for MRB_CPP_DEFINE_TYPE
// ----------------------------------------------

struct BaseTest {
  int v;
  BaseTest(int x) : v(x) {}
  virtual ~BaseTest() = default;
};

struct DerivedTest : BaseTest {
  DerivedTest(int x) : BaseTest(x) {}
};

struct MoreDerivedTest : DerivedTest {
  MoreDerivedTest(int x) : DerivedTest(x) {}
};

// Register only the BASE class
MRB_CPP_DEFINE_TYPE(BaseTest, basetest)

static void run_subclassing_tests(mrb_state* mrb) {
  // --- Base ---
  mrb_value o1 = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb->object_class));
  BaseTest* b = mrb_cpp_new<BaseTest>(mrb, o1, 10);
  assert(b->v == 10);
  assert(DATA_TYPE(o1) == &basetest_type);

  // --- Derived ---
  mrb_value o2 = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb->object_class));
  DerivedTest* d = mrb_cpp_new<DerivedTest>(mrb, o2, 20);
  assert(d->v == 20);
  assert(DATA_TYPE(o2) == &basetest_type);  // same type!

  // --- MoreDerived ---
  mrb_value o3 = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb->object_class));
  MoreDerivedTest* md = mrb_cpp_new<MoreDerivedTest>(mrb, o3, 30);
  assert(md->v == 30);
  assert(DATA_TYPE(o3) == &basetest_type);  // same type!

  // --- mrb_data_get_ptr works for all ---
  BaseTest* b1 = (BaseTest*)mrb_data_get_ptr(mrb, o1, &basetest_type);
  BaseTest* b2 = (BaseTest*)mrb_data_get_ptr(mrb, o2, &basetest_type);
  BaseTest* b3 = (BaseTest*)mrb_data_get_ptr(mrb, o3, &basetest_type);

  assert(b1->v == 10);
  assert(b2->v == 20);
  assert(b3->v == 30);

  // --- Name is correct (namespace stripped) ---
  assert(std::string(basetest_type.struct_name) == "BaseTest");
}

static mrb_value
subclassing_ok_q(mrb_state* mrb, mrb_value self)
{
  run_subclassing_tests(mrb);
  return mrb_true_value();
}

static void test_edges(mrb_state* mrb) {
  // --- Fixnum boundaries ---
  {
    mrb_value v = mrb_convert_number(mrb, MRB_FIXNUM_MIN);
    assert(mrb_fixnum_p(v));
    assert(mrb_fixnum(v) == MRB_FIXNUM_MIN);

    v = mrb_convert_number(mrb, MRB_FIXNUM_MAX);
    assert(mrb_fixnum_p(v));
    assert(mrb_fixnum(v) == MRB_FIXNUM_MAX);

#if defined(__SIZEOF_INT128__)
    // Just outside fixnum range
    auto over_fix = (__int128)MRB_FIXNUM_MAX + 1;
    v = mrb_convert_number(mrb, over_fix);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == over_fix);

    auto under_fix = (__int128)MRB_FIXNUM_MIN - 1;
    v = mrb_convert_number(mrb, under_fix);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == under_fix);
#endif
  }

  // --- mrb_int boundaries ---
  {
    mrb_value v = mrb_convert_number(mrb, MRB_INT_MIN);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MIN);

    v = mrb_convert_number(mrb, MRB_INT_MAX);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MAX);

#if defined(__SIZEOF_INT128__) && defined(MRB_USE_BIGINT)
    auto over_int = (__int128)MRB_INT_MAX + 1;
    v = mrb_convert_number(mrb, over_int);
    assert(mrb_bigint_p(v));

    auto under_int = (__int128)MRB_INT_MIN - 1;
    v = mrb_convert_number(mrb, under_int);
    assert(mrb_bigint_p(v));
#endif
  }

  // --- Unsigned types ---
  {
    mrb_value v = mrb_convert_number(mrb, uint64_t{0});
    assert(mrb_fixnum_p(v));
    assert(mrb_fixnum(v) == 0);

    v = mrb_convert_number(mrb, static_cast<uint64_t>(MRB_INT_MAX));
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MAX);

#if defined(__SIZEOF_INT128__) && defined(MRB_USE_BIGINT)
    auto over_uint = static_cast<uint64_t>((__int128)MRB_INT_MAX + 1);
    v = mrb_convert_number(mrb, over_uint);
    assert(mrb_bigint_p(v));

    v = mrb_convert_number(mrb, std::numeric_limits<uint64_t>::max());
    assert(mrb_bigint_p(v));
#endif
  }

  // --- Signed types ---
  {
    mrb_value v = mrb_convert_number(mrb, int64_t{MRB_INT_MIN});
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MIN);

    v = mrb_convert_number(mrb, int64_t{MRB_INT_MAX});
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MAX);

#if defined(__SIZEOF_INT128__) && defined(MRB_USE_BIGINT)
    auto over_sint = (__int128)MRB_INT_MAX + 1;
    v = mrb_convert_number(mrb, over_sint);
    assert(mrb_bigint_p(v));

    auto under_sint = (__int128)MRB_INT_MIN - 1;
    v = mrb_convert_number(mrb, under_sint);
    assert(mrb_bigint_p(v));

    // The value, not only the type: 2**100, its negative, and the two
    // ends of the 128-bit range, read back as decimal.
    auto says = [&](mrb_value x, const char* want) {
      mrb_value s = mrb_integer_to_str(mrb, x, 10);
      assert(std::string(RSTRING_PTR(s), RSTRING_LEN(s)) == want);
    };
    says(mrb_convert_number(mrb, (unsigned __int128)1 << 100), "1267650600228229401496703205376");
    says(mrb_convert_number(mrb, -((__int128)1 << 100)), "-1267650600228229401496703205376");
    says(mrb_convert_number(mrb, std::numeric_limits<unsigned __int128>::max()),
         "340282366920938463463374607431768211455");
    says(mrb_convert_number(mrb, std::numeric_limits<__int128>::min()),
         "-170141183460469231731687303715884105728");
    says(mrb_convert_number(mrb, (__int128)0), "0");
#endif
  }
}

static mrb_value
numeric_edges_ok_q(mrb_state* mrb, mrb_value self)
{
  test_edges(mrb);
  return mrb_true_value();
}

// -------------------------------------------------------------
// Test: mrb_cpp_new + mrb_cpp_get round-trip
// -------------------------------------------------------------

struct TestThing {
  int x;
  int y;

  TestThing(int a, int b) : x(a), y(b) {}
  ~TestThing() = default;
};

MRB_CPP_DEFINE_TYPE(TestThing, testthing)

static void run_cpp_data_roundtrip_test(mrb_state* mrb) {
  // Define a Ruby class to hold the DATA object
  struct RClass* cls =
      mrb_define_class(mrb, "TestThingHolder", mrb->object_class);
  MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);

  // Define initialize that constructs TestThing(10, 20)
  mrb_define_method(
      mrb, cls, "initialize",
      [](mrb_state* mrb, mrb_value self) -> mrb_value {
        mrb_cpp_new<TestThing>(mrb, self, 10, 20);
        return self;
      },
      MRB_ARGS_NONE()
  );

  // Create a Ruby object
  mrb_value obj = mrb_obj_new(mrb, cls, 0, nullptr);

  // Retrieve the C++ instance
  TestThing* ptr = mrb_cpp_get<TestThing>(mrb, obj);

  // Validate
  assert(ptr != nullptr);
  assert(ptr->x == 10);
  assert(ptr->y == 20);
}

static mrb_value
cpp_data_roundtrip_ok_q(mrb_state* mrb, mrb_value self)
{
  run_cpp_data_roundtrip_test(mrb);
  return mrb_true_value();
}

MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb) {
  struct RClass* m = mrb_define_module(mrb, "CExtHelpersVectors");

  mrb_define_module_function(mrb, m, "any_roundtrip", any_roundtrip, MRB_ARGS_REQ(1));

  mrb_define_module_function(mrb, m, "to_mrb_bool", to_mrb_bool, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, m, "to_mrb_int", to_mrb_int, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, m, "to_mrb_double", to_mrb_double, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, m, "to_mrb_std_string", to_mrb_std_string, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_string_view", to_mrb_string_view, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_cstr", to_mrb_cstr, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_nullptr", to_mrb_nullptr, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_map", to_mrb_map, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_unordered_map", to_mrb_unordered_map, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_set", to_mrb_set, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_unordered_set", to_mrb_unordered_set, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_vector", to_mrb_vector, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_array", to_mrb_array, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "to_mrb_time", to_mrb_time, MRB_ARGS_NONE());

  mrb_define_module_function(mrb, m, "numeric_edges_ok?", numeric_edges_ok_q, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "subclassing_ok?", subclassing_ok_q, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "cpp_data_roundtrip_ok?", cpp_data_roundtrip_ok_q, MRB_ARGS_NONE());
}
MRB_END_DECL
