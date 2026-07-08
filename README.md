# mruby-c-ext-helpers

This mgem includes helpers for c/c++ extenstion i use frenquently, but are lacking from the core mruby distribution, namely number encoding and decoding, helping with deciding which type of Numeric to pick (Integer, Float, Bigint) etc.
Take a look at the test/cpp_tests.cpp file on how to use some of them, or the test.rb file there.

You need a c++17 compatible compiler to build this.

## C++ standard requirements

The C++ headers of this gem (`num_helpers.hpp`, `cpp_helpers.hpp`, `cpp_to_mrb_value.hpp`, `mrb_value_to_cpp.hpp`) require **at least C++17**, and that requirement applies to *every translation unit that includes them* — i.e. also to dependent gems, not just to this gem's own sources. Compiling below C++17 fails fast with a single `#error` from the header.

The flag each toolchain needs:

| Toolchain / platform | Flag | Compiler default without it |
|---|---|---|
| gcc (Linux, MinGW) | `-std=c++17` | gnu++17 on gcc 11+ — works by accident |
| clang / LLVM (Linux, BSD) | `-std=c++17` | gnu++17 on clang 16+ — works by accident |
| Apple clang (macOS) | `-std=c++17` | **older than C++17 — fails without the flag** |
| MSVC (Windows) | `/std:c++17` | **C++14 — fails without the flag** |

Loading this gem sets that floor build-wide automatically, so in the common case you don't have to do anything. Two details are worth knowing:

- The floor is strict `c++17`, **not** `gnu++17` — if your code relies on GNU extensions or POSIX prototypes hidden behind feature-test macros, define them yourself (e.g. `_DEFAULT_SOURCE`).
- mruby's build system never propagates one gem's `spec.cxx.flags` to another gem, which is why the floor has to be applied build-wide. Anything you add yourself in the places below only affects that one place.

Where and how to add a `-std` flag yourself, e.g. when your gem wants something newer than the floor (a flag you add is always respected — the floor skips gems that already carry a `-std` flag, and a later `-std` on the command line wins anyway):

In your gem's `mrbgem.rake`, for your gem's own sources:

```ruby
MRuby::Gem::Specification.new('my-gem') do |spec|
  spec.add_dependency 'mruby-c-ext-helpers'
  if spec.for_windows?
    spec.cxx.flags << '/std:c++20'
  else
    spec.cxx.flags << '-std=c++20'
  end
end
```

In a `build_config.rb`, for every C++ translation unit of that build:

```ruby
MRuby::Build.new do |conf|
  toolchain :gcc
  conf.cxx.flags << '-std=c++20'
  conf.gem mgem: 'mruby-c-ext-helpers'
end
```

Same thing for a Windows build with the Visual C++ toolchain:

```ruby
MRuby::Build.new do |conf|
  toolchain :visualcpp
  conf.cxx.flags << '/std:c++20'
  conf.gem mgem: 'mruby-c-ext-helpers'
end
```

And identically inside a `MRuby::CrossBuild` block for cross-compilation:

```ruby
MRuby::CrossBuild.new('mytarget') do |conf|
  toolchain :clang
  conf.cxx.flags << '-std=c++20'
  conf.gem mgem: 'mruby-c-ext-helpers'
end
```

One MSVC caveat when checking the standard in your own code: MSVC reports `__cplusplus` as `199711L` unless you also pass `/Zc:__cplusplus` — check `_MSVC_LANG` instead (this gem's header guard already does).

Sample code to wrap calling new and delete on a c++ class

```c++
#include <mruby/cpp_helpers.hpp>
MRB_CPP_DEFINE_TYPE(ClassName, UniqueIdentifier)
//add the above outside a function call

mrb_cpp_new<ClassName>(mrb, self, ...);
```

Put that in the initialize method of a mruby class which has the MRB_TT_DATA type and mruby will manage the lifetime of your c++ Object. Arguments will get forwarded to the new method of your c++ class.


Convert most c++ values to mruby objects:

```c++
#include <mruby/cpp_to_mrb_value.hpp>
std::vector<int> v = {1, 2, 3};
mrb_value arr = cpp_to_mrb_value(mrb, v);
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

convert most mruby objects to c++ values
```c++
#include <mruby/mrb_value_to_cpp.hpp>

mrb_value s = mrb_str_new_lit(mrb, "hello");
std::any as = mrb_value_to_any(mrb, s);
assert(std::any_cast<std::string>(as) == "hello");
```
this exposes the following functions:
```c++
MRB_API std::any mrb_value_to_any(mrb_state* mrb, mrb_value val);
MRB_API std::vector<std::any> mrb_array_to_vector(mrb_state* mrb, mrb_value ary);

// Map keys can be int64_t, double, or string
using MapKey = std::variant<mrb_int, mrb_float, std::string>;
MRB_API std::map<MapKey, std::any> mrb_hash_to_map(mrb_state* mrb, mrb_value hash);
```
