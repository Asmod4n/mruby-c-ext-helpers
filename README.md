# mruby-c-ext-helpers

This mgem includes helpers for c/c++ extenstion i use frenquently, but are lacking from the core mruby distribution, namely number encoding and decoding, helping with deciding which type of Numeric to pick (Integer, Float, Bigint) etc.
Take a look at the test/cpp_tests.cpp file on how to use some of them, or the test.rb file there.

You need a c++17 compatible compiler to build this.

Sample code to wrap calling new and delete on a c++ class

```c++
MRB_CPP_DEFINE_TYPE(ClassName, UniqueIdenifier)

mrb_cpp_new<ClassName>(mrb, self, ...);
```

Put that in the initialize method of a mruby class which has the MRB_TT_DATA type and mruby will manage the lifetime of your c++ Object. Arguments will get forwarded to the new method of your c++ class.
