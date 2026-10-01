#include <stdexcept>
/*
 * Test-only Ruby surface over this gem's C++ helpers, compiled into
 * mrbtest and nothing else. The conversion between C++ values and
 * mruby values moved to mruby-cpp, and its tests went with it.
 *
 * The checks here (numeric edge cases, the MRB_CPP_DEFINE_TYPE
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
#include <mruby/cpp_helpers.hpp>
#include <mruby/num_helpers.hpp>
#include <mruby/numeric.h>

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

    // Just outside fixnum range. Where the build boxes a value, an
    // mrb_int is wider than a fixnum and holds these two.
    if constexpr (MRB_FIXNUM_MAX < MRB_INT_MAX) {
      auto over_fix = static_cast<int64_t>(MRB_FIXNUM_MAX) + 1;
      v = mrb_convert_number(mrb, over_fix);
      assert(mrb_integer_p(v));
      assert(mrb_integer(v) == over_fix);

      auto under_fix = static_cast<int64_t>(MRB_FIXNUM_MIN) - 1;
      v = mrb_convert_number(mrb, under_fix);
      assert(mrb_integer_p(v));
      assert(mrb_integer(v) == under_fix);
    }
  }

  // --- mrb_int boundaries ---
  {
    mrb_value v = mrb_convert_number(mrb, MRB_INT_MIN);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MIN);

    v = mrb_convert_number(mrb, MRB_INT_MAX);
    assert(mrb_integer_p(v));
    assert(mrb_integer(v) == MRB_INT_MAX);

#ifdef MRB_USE_BIGINT
    // Above mrb_int there is only the unsigned side to come from: a
    // signed C type that reaches past MRB_INT_MAX does not exist here.
    auto over_int = static_cast<uint64_t>(MRB_INT_MAX) + 1;
    v = mrb_convert_number(mrb, over_int);
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

#ifdef MRB_USE_BIGINT
    auto over_uint = static_cast<uint64_t>(MRB_INT_MAX) + 1;
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

#ifdef MRB_USE_BIGINT
    // The value, not only the type: the two ends that reach past
    // mrb_int, read back as decimal.
    auto says = [&](mrb_value x, const char* want) {
      mrb_value s = mrb_integer_to_str(mrb, x, 10);
      assert(std::string(RSTRING_PTR(s), RSTRING_LEN(s)) == want);
    };
    says(mrb_convert_number(mrb, std::numeric_limits<uint64_t>::max()),
         "18446744073709551615");
    says(mrb_convert_number(mrb, static_cast<uint64_t>(MRB_INT_MAX) + 1),
         "9223372036854775808");
    says(mrb_convert_number(mrb, int64_t{MRB_INT_MIN}), "-9223372036854775808");
    says(mrb_convert_number(mrb, uint64_t{0}), "0");
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
test_thing_sum(mrb_state* mrb, mrb_value self)
{
  mrb_value obj;
  mrb_get_args(mrb, "o", &obj);
  const TestThing* t = mrb_cpp_get<TestThing>(mrb, obj);
  return mrb_int_value(mrb, t->x + t->y);
}

static mrb_value
cpp_data_roundtrip_ok_q(mrb_state* mrb, mrb_value self)
{
  run_cpp_data_roundtrip_test(mrb);
  return mrb_true_value();
}

// -------------------------------------------------------------
// mrb_cpp_new and mrb_cpp_delete free each object exactly once.
// These run under ASan, which reports a double free, a leak or a
// free of a wrong address.
// -------------------------------------------------------------

struct ThrowingThing {
  ThrowingThing() { throw std::runtime_error("constructor fails"); }
};

MRB_CPP_DEFINE_TYPE(ThrowingThing, throwingthing)

// A constructor that throws must leave the object without data and
// without a type, so that the collector never calls dfree for it.
static mrb_value
throwing_constructor_leaves_no_data(mrb_state* mrb, mrb_value self)
{
  struct RClass* cls = mrb_class_get(mrb, "ThrowingThingHolder");
  mrb_value obj = mrb_obj_value(mrb_data_object_alloc(mrb, cls, nullptr, nullptr));
  bool thrown = false;
  try {
    mrb_cpp_new<ThrowingThing>(mrb, obj);
  } catch (const std::runtime_error&) {
    thrown = true;
  }
  return mrb_bool_value(thrown && DATA_PTR(obj) == nullptr && DATA_TYPE(obj) == nullptr);
}

struct Mixin {
  int m = 7;
  virtual ~Mixin() = default;
};

struct PolyBase {
  int v;
  explicit PolyBase(int x) : v(x) {}
  virtual ~PolyBase() = default;
};

// PolyBase is the second base, so a PolyBase* does not point at the
// start of the allocation.
struct MultiDerived : Mixin, PolyBase {
  explicit MultiDerived(int x) : PolyBase(x) {}
};

MRB_CPP_DEFINE_TYPE(PolyBase, polybase)

static mrb_value
multi_derived_value(mrb_state* mrb, mrb_value self)
{
  mrb_value obj;
  mrb_get_args(mrb, "o", &obj);
  return mrb_int_value(mrb, mrb_cpp_get<PolyBase>(mrb, obj)->v);
}

MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb) {
  struct RClass* m = mrb_define_module(mrb, "CExtHelpersVectors");

  struct RClass* holder = mrb_define_class(mrb, "TestThingHolder", mrb->object_class);
  MRB_SET_INSTANCE_TT(holder, MRB_TT_DATA);
  mrb_define_method(mrb, holder, "initialize",
      [](mrb_state* mrb, mrb_value self) -> mrb_value {
        mrb_cpp_new<TestThing>(mrb, self, 10, 20);
        return self;
      },
      MRB_ARGS_NONE());

  struct RClass* throwing_holder = mrb_define_class(mrb, "ThrowingThingHolder", mrb->object_class);
  MRB_SET_INSTANCE_TT(throwing_holder, MRB_TT_DATA);
  mrb_define_module_function(mrb, m, "throwing_constructor_leaves_no_data", throwing_constructor_leaves_no_data, MRB_ARGS_NONE());
  struct RClass* multi_holder = mrb_define_class(mrb, "MultiDerivedHolder", mrb->object_class);
  MRB_SET_INSTANCE_TT(multi_holder, MRB_TT_DATA);
  mrb_define_method(mrb, multi_holder, "initialize",
      [](mrb_state* mrb, mrb_value self) -> mrb_value {
        mrb_cpp_new<MultiDerived>(mrb, self, 42);
        return self;
      },
      MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "multi_derived_value", multi_derived_value, MRB_ARGS_REQ(1));

  mrb_define_module_function(mrb, m, "numeric_edges_ok?", numeric_edges_ok_q, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "subclassing_ok?", subclassing_ok_q, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, m, "test_thing_sum", test_thing_sum, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, m, "cpp_data_roundtrip_ok?", cpp_data_roundtrip_ok_q, MRB_ARGS_NONE());
}
MRB_END_DECL
