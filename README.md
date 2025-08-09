# mruby-c-ext-helpers

This mgem includes helpers for c/c++ extenstion i use frenquently, but are lacking from the core mruby distribution, namely number encoding and decoding, helping with deciding which type of Numeric to pick (Integer, Float, Bigint) etc.
Take a look at the test/cpp_tests.cpp file on how to use some of them, or the test.rb file there.

You need a c++17 compatible compiler to build this.

Sample code to wrap calling new and delete on a c++ class

```c++
#include <mruby/cpp_helpers.hpp>
MRB_CPP_DEFINE_TYPE(ClassName, UniqueIdentifier)

mrb_cpp_new<ClassName>(mrb, self, ...);
```

Put that in the initialize method of a mruby class which has the MRB_TT_DATA type and mruby will manage the lifetime of your c++ Object. Arguments will get forwarded to the new method of your c++ class.


Convert most c++ values to mruby objects:

```c++
#include <mruby/mrb_convert_cpp_value.hpp>
std::vector<int> v = {1, 2, 3};
mrb_value arr = mrb_convert_cpp_value(mrb, v);
assert(mrb_type(arr) == MRB_TT_ARRAY);
assert(RARRAY_LEN(arr) == 3);
assert(mrb_integer(mrb_ary_ref(mrb, arr, 0)) == 1);
```
This works with numbers, maps, sets, strings, vectors and a few more which can be represented in mruby.


convert most c numeric types to an mruby number:
```c
#include <mruby/num_helpers.h>

size_t n = 15;
mrb_value number = mrb_convert_size_t(mrb, n);

long long l = -15;
number = mrb_convert_long_long(mrb, l);
```