assert("Native Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin.to_fix)
end

assert("Little Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_le.to_fix_le)
end

assert("Big Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_be.to_fix_be)
end
