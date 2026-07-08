MRuby::Gem::Specification.new('mruby-c-ext-helpers') do |spec|
  spec.license = 'MPL-2'
  spec.authors = 'Hendrik Beskow'
  spec.version = "0.3.0"
  spec.add_test_dependency 'mruby-set'
  spec.add_test_dependency 'mruby-time'
  spec.add_test_dependency 'mruby-bigint'
  spec.add_test_dependency 'mruby-struct'
  spec.add_test_dependency 'mruby-compiler'
  # num_helpers.hpp is consumed by DEPENDENT gems' translation units, and
  # spec.cxx.flags never propagate to dependents, so the C++17 floor must be
  # set build-wide. Compiler defaults differ (Apple clang predates C++17,
  # so its default breaks the constexpr helpers in the header).
  if spec.for_windows?
    spec.build.cxx.flags << '/std:c++17'
  else
    spec.build.cxx.flags << '-std=c++17'
  end
end
