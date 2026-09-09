MRuby::Gem::Specification.new('mruby-c-ext-helpers') do |spec|
  spec.license = 'MPL-2'
  spec.authors = 'Hendrik Beskow'
  spec.version = "0.3.0"
  spec.add_test_dependency 'mruby-set'
  spec.add_test_dependency 'mruby-time'
  spec.add_test_dependency 'mruby-bigint'
  spec.add_test_dependency 'mruby-struct'
  # num_helpers.hpp is consumed by DEPENDENT gems' translation units, and
  # gem-level cxx flags never propagate to dependents, so the C++17 floor
  # must be set build-wide. Compiler defaults differ (Apple clang predates
  # C++17, so its default breaks the constexpr helpers in the header).
  #
  # Every gem's setup deep-clones the build's compilers BEFORE its
  # mrbgem.rake block runs, so this block must patch both directions:
  # the build's cxx (inherited by every compiler cloned from now on) and
  # the clones that already exist (this gem's own, and any gem set up
  # earlier). Gems that pick their own -std are left untouched; gems that
  # append one later still win because the last -std on the line wins.
  # Which spelling, decided by the compiler and not by the platform.
  # for_windows? is true for a MinGW cross build as well, and MinGW is
  # gcc: it wants -std=c++17 and reads /std:c++17 as a file name.
  is_msvc = spec.build.toolchains.include?('visualcpp') ||
            spec.build.cxx.command.to_s =~ %r{(^|[\\/])cl(\.exe)?$}i
  std_flag = is_msvc ? '/std:c++17' : '-std=c++17'
  has_std = ->(cxx) { cxx.flags.flatten.any? { |f| f.to_s.match?(%r{[-/]std[:=]}) } }
  spec.build.cxx.flags << std_flag unless has_std.call(spec.build.cxx)
  spec.build.gems.each do |g|
    g.cxx.flags << std_flag unless g.cxx.nil? || has_std.call(g.cxx)
  end
end
