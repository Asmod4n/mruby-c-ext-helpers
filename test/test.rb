assert("Native Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin.to_fix)
end

assert("Little Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_le.to_fix_le)
end

assert("Big Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_be.to_fix_be)
end

# Two values in an order-independent pair, without relying on
# Array#sort's element order for the unordered container tests below.
def unordered_pair?(ary, a, b)
  ary.size == 2 && ((ary[0] == a && ary[1] == b) || (ary[0] == b && ary[1] == a))
end

# --- Pure C++/C-API checks with no Ruby-observable output beyond
# "it ran to completion": numeric edge cases, the MRB_CPP_DEFINE_TYPE
# subclassing contract, and the mrb_cpp_new/mrb_cpp_get round trip.

assert("numeric_edges_ok? -- mrb_convert_number boundary values") do
  assert_true(CExtHelpersVectors.numeric_edges_ok?)
end

assert("subclassing_ok? -- MRB_CPP_DEFINE_TYPE shares one mrb_data_type across a hierarchy") do
  assert_true(CExtHelpersVectors.subclassing_ok?)
end

assert("cpp_data_roundtrip_ok? -- mrb_cpp_new / mrb_cpp_get round trip") do
  assert_true(CExtHelpersVectors.cpp_data_roundtrip_ok?)
end

# initialize can be called again from Ruby. A second call must not
# replace the C++ object, because the first one would leak and never
# be destroyed.
assert("mrb_cpp_new refuses a second initialize") do
  holder = TestThingHolder.new
  assert_raise(TypeError) { holder.__send__(:initialize) }
  assert_equal(30, CExtHelpersVectors.test_thing_sum(holder))
end

# The memory of a failed constructor is freed by mrb_cpp_new, and the
# object gets no data type, so dfree does not free it a second time.
assert("mrb_cpp_new frees the memory once when the constructor throws") do
  assert_true(CExtHelpersVectors.throwing_constructor_leaves_no_data)
  GC.start
end

# Every object is freed by its dfree once. ASan reports a second free.
assert("mrb_cpp_new objects are freed once by the collector") do
  100.times { TestThingHolder.new }
  GC.start
end

# A class with two polymorphic bases, registered through its second
# base. Reading and freeing must use the address of that base, and
# the free must reach the start of the allocation.
assert("MRB_CPP_DEFINE_TYPE frees a subclass with two bases once") do
  holder = MultiDerivedHolder.new
  assert_equal(42, CExtHelpersVectors.multi_derived_value(holder))
  holder = nil
  GC.start
end
