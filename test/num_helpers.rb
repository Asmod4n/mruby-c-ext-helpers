assert("Native Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin.to_fix)
end

assert("Little Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_le.to_fix_le)
end

assert("Big Endian Fixnum de-/encoding") do
  assert_equal(100, 100.to_bin_be.to_fix_be)
end

if Object.const_defined?("Float")
  assert("Native Float de-/encoding") do
    assert_equal(100.1, 100.1.to_bin.to_flo)
  end

  assert("Little Endian Float de-/encoding") do
    assert_equal(100.1, 100.1.to_bin_le.to_flo_le)
  end

  assert("Big Endian Float de-/encoding") do
    assert_equal(100.1, 100.1.to_bin_be.to_flo_be)
  end
end
