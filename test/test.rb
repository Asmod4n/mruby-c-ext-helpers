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

# --- mrb_value_to_any / mrb_array_to_vector / mrb_hash_to_map, driven
# through CExtHelpersVectors.any_roundtrip, which is mrb_value_to_any
# followed by the inverse conversion back to mrb_value. Every literal
# below is built by the build-time-compiled bytecode of this file, not
# by any runtime compiler.

assert("any_roundtrip: Integer") do
  assert_equal(42, CExtHelpersVectors.any_roundtrip(42))
end

assert("any_roundtrip: Float") do
  assert_equal(3.14, CExtHelpersVectors.any_roundtrip(3.14))
end

assert("any_roundtrip: Boolean") do
  assert_equal(true, CExtHelpersVectors.any_roundtrip(true))
  assert_equal(false, CExtHelpersVectors.any_roundtrip(false))
end

assert("any_roundtrip: nil maps to an empty std::any") do
  assert_nil(CExtHelpersVectors.any_roundtrip(nil))
end

assert("any_roundtrip: String") do
  assert_equal("hello", CExtHelpersVectors.any_roundtrip("hello"))
end

assert("any_roundtrip: Symbol becomes its name as a String") do
  assert_equal("sym", CExtHelpersVectors.any_roundtrip(:sym))
end

assert("any_roundtrip: Array") do
  assert_equal([1, 2, 3], CExtHelpersVectors.any_roundtrip([1, 2, 3]))
end

assert("any_roundtrip: Struct is handled like an Array (MRB_TT_STRUCT)") do
  s = Struct.new(:a, :b).new(1, 2)
  assert_equal([1, 2], CExtHelpersVectors.any_roundtrip(s))
end

assert("any_roundtrip: Hash") do
  assert_equal({ "a" => 1, "b" => 2 }, CExtHelpersVectors.any_roundtrip({ "a" => 1, "b" => 2 }))
end

assert("any_roundtrip: Set goes through to_a (MRB_TT_SET)") do
  result = CExtHelpersVectors.any_roundtrip(Set["x", "y"])
  assert_true(unordered_pair?(result, "x", "y"))
end

# --- cpp_to_mrb_value<T>, one C++-side input per probe, checked here.

assert("to_mrb_bool") do
  assert_equal(true, CExtHelpersVectors.to_mrb_bool(true))
  assert_equal(false, CExtHelpersVectors.to_mrb_bool(false))
end

assert("to_mrb_int") do
  assert_equal(123, CExtHelpersVectors.to_mrb_int(123))
end

assert("to_mrb_double") do
  assert_equal(3.1415, CExtHelpersVectors.to_mrb_double(3.1415))
end

assert("to_mrb_std_string") do
  assert_equal("foo", CExtHelpersVectors.to_mrb_std_string)
end

assert("to_mrb_string_view") do
  assert_equal("bar", CExtHelpersVectors.to_mrb_string_view)
end

assert("to_mrb_cstr") do
  assert_equal("baz", CExtHelpersVectors.to_mrb_cstr)
end

assert("to_mrb_nullptr maps to nil") do
  assert_nil(CExtHelpersVectors.to_mrb_nullptr)
end

assert("to_mrb_map (map-like)") do
  h = CExtHelpersVectors.to_mrb_map
  assert_equal(1, h["a"])
  assert_equal(2, h["b"])
end

assert("to_mrb_unordered_map (map-like)") do
  h = CExtHelpersVectors.to_mrb_unordered_map
  assert_equal(true, h["x"])
  assert_equal(false, h["y"])
end

assert("to_mrb_set (set-like) is a Ruby Set") do
  s = CExtHelpersVectors.to_mrb_set
  assert_true(s.kind_of?(Set))
  assert_true(unordered_pair?(s.to_a, 10, 20))
end

assert("to_mrb_unordered_set (set-like) is a Ruby Set") do
  s = CExtHelpersVectors.to_mrb_unordered_set
  assert_true(s.kind_of?(Set))
  assert_true(unordered_pair?(s.to_a, "foo", "bar"))
end

assert("to_mrb_vector (iterable container)") do
  assert_equal([1, 2, 3], CExtHelpersVectors.to_mrb_vector)
end

assert("to_mrb_array (iterable container)") do
  assert_equal(["x", "y"], CExtHelpersVectors.to_mrb_array)
end

assert("to_mrb_time is a Ruby Time") do
  assert_true(CExtHelpersVectors.to_mrb_time.kind_of?(Time))
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
