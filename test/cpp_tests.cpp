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
#include <mruby/compile.h>

static void run_value_to_cpp_tests(mrb_state* mrb) {
  // --- Scalars ---
  mrb_value i = mrb_load_string(mrb, "42");
  std::any ai = mrb_value_to_any(mrb, i);
  assert(std::any_cast<mrb_int>(ai) == 42);

  mrb_value f = mrb_load_string(mrb, "3.14");
  std::any af = mrb_value_to_any(mrb, f);
  assert(std::abs(std::any_cast<mrb_float>(af) - 3.14) < 1e-6);

  // --- Boolean ---
  mrb_value t = mrb_load_string(mrb, "true");
  mrb_value fval = mrb_load_string(mrb, "false");
  assert(std::any_cast<bool>(mrb_value_to_any(mrb, t)) == true);
  assert(std::any_cast<bool>(mrb_value_to_any(mrb, fval)) == false);

  // --- Nil ---
  mrb_value n = mrb_load_string(mrb, "nil");
  std::any an = mrb_value_to_any(mrb, n);
  assert(!an.has_value());

  // --- String ---
  mrb_value s = mrb_load_string(mrb, "'hello'");
  std::any as = mrb_value_to_any(mrb, s);
  assert(std::any_cast<std::string>(as) == "hello");

  // --- Symbol ---
  mrb_value sym = mrb_load_string(mrb, ":sym");
  std::any sym_any = mrb_value_to_any(mrb, sym);
  assert(std::any_cast<std::string>(sym_any) == "sym");

  // --- Array ---
  mrb_value arr = mrb_load_string(mrb, "[1, 2, 3]");
  std::any avec = mrb_value_to_any(mrb, arr);
  auto vec_out = std::any_cast<std::vector<std::any>>(avec);
  assert(std::any_cast<mrb_int>(vec_out[0]) == 1);

  // --- Struct ---
  mrb_value struct_val = mrb_load_string(mrb,
    "Foo = Struct.new(:a, :b); Foo.new(1, 2)");
  std::any svec = mrb_value_to_any(mrb, struct_val);
  auto svec_out = std::any_cast<std::vector<std::any>>(svec);
  assert(std::any_cast<mrb_int>(svec_out[0]) == 1);

  // --- Hash ---
  mrb_value h = mrb_load_string(mrb, "{'a' => 1, 'b' => 2}");
  std::any ah = mrb_value_to_any(mrb, h);
  auto map_out = std::any_cast<std::map<MapKey,std::any>>(ah);
  assert(std::any_cast<mrb_int>(map_out[std::string("a")]) == 1);

  // --- Set ---
  // needs 'set' library loaded in mruby
  mrb_value ruby_set = mrb_load_string(mrb, "Set['x', 'y']");
  std::any aset = mrb_value_to_any(mrb, ruby_set);
  auto set_vec = std::any_cast<std::vector<std::any>>(aset);
  assert(set_vec.size() == 2);
}

static void run_cpp_to_mrb_tests(mrb_state* mrb) {
    using namespace std::chrono;

    // --- Booleans ---
    mrb_value t = cpp_to_mrb_value(mrb, true);
    mrb_value f = cpp_to_mrb_value(mrb, false);
    assert(mrb_bool(t));
    assert(!mrb_bool(f));

    // --- Arithmetic ---
    mrb_value i = cpp_to_mrb_value(mrb, 123);
    assert(mrb_integer(i) == 123);
    mrb_value d = cpp_to_mrb_value(mrb, 3.1415);
    assert(std::abs(mrb_float(d) - 3.1415) < 1e-6);

    // --- Strings ---
    mrb_value s1 = cpp_to_mrb_value(mrb, std::string("foo"));
    mrb_value s2 = cpp_to_mrb_value(mrb, std::string_view("bar"));
    mrb_value s3 = cpp_to_mrb_value(mrb, "baz");
    assert(std::string(RSTRING_PTR(s1), RSTRING_LEN(s1)) == "foo");
    assert(std::string(RSTRING_PTR(s2), RSTRING_LEN(s2)) == "bar");
    assert(std::string(RSTRING_PTR(s3), RSTRING_LEN(s3)) == "baz");

    // --- nullptr ---
    mrb_value n = cpp_to_mrb_value(mrb, nullptr);
    assert(mrb_nil_p(n));

    // --- map-like ---
    std::map<std::string,int> smap = {{"a",1},{"b",2}};
    mrb_value hash_val = cpp_to_mrb_value(mrb, smap);
    assert(mrb_type(hash_val) == MRB_TT_HASH);
    assert(mrb_integer(mrb_hash_get(mrb, hash_val, mrb_str_new_lit(mrb,"a"))) == 1);

    std::unordered_map<std::string,bool> umap = {{"x",true},{"y",false}};
    mrb_value uval = cpp_to_mrb_value(mrb, umap);
    assert(mrb_bool(mrb_hash_get(mrb, uval, mrb_str_new_lit(mrb,"x"))));

    // --- set-like ---
    std::set<int> sset = {10,20};
    mrb_value sset_val = cpp_to_mrb_value(mrb, sset);
    struct RClass* set_cls = mrb_class_get_id(mrb, MRB_SYM(Set));
    assert(mrb_obj_is_kind_of(mrb, sset_val, set_cls));

    std::unordered_set<std::string> uset = {"foo","bar"};
    mrb_value uset_val = cpp_to_mrb_value(mrb, uset);
    assert(mrb_obj_is_kind_of(mrb, uset_val, set_cls));

    // --- iterable containers ---
    std::vector<int> v = {1,2,3};
    mrb_value vval = cpp_to_mrb_value(mrb, v);
    assert(mrb_type(vval) == MRB_TT_ARRAY);

    std::array<std::string,2> arr = {"x","y"};
    mrb_value aval = cpp_to_mrb_value(mrb, arr);
    assert(mrb_type(aval) == MRB_TT_ARRAY);

    // --- time_point ---
    auto now = system_clock::now();
    mrb_value tval = cpp_to_mrb_value(mrb, now);
    struct RClass* time_cls = mrb_class_get_id(mrb, MRB_SYM(Time));
    assert(mrb_obj_is_kind_of(mrb, tval, time_cls));
}
MRB_CPP_DEFINE_TYPE(std::string, stdstring)

void test_edges(mrb_state* mrb) {
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
#endif
  }
}

// -------------------------------------------------------------
// Test: mrb_cpp_new + mrb_cpp_get round‑trip
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


MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb) {
    run_value_to_cpp_tests(mrb);
    run_cpp_to_mrb_tests(mrb);
    test_edges(mrb);
    run_cpp_data_roundtrip_test(mrb);
}
MRB_END_DECL
