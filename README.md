# mruby-c-ext-helpers

This mgem includes helpers for c/c++ extenstion i use frenquently, but are lacking from the core mruby distribution, namely number encoding and decoding, helping with deciding which type of Numeric to pick (Integer, Float, Bigint) etc.
Take a look at the test/cpp_tests.cpp file on how to use some of them, or the test.rb file there.

You need a c++17 compatible compiler to build this.