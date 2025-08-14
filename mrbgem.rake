MRuby::Gem::Specification.new('mruby-c-ext-helpers') do |spec|
  spec.license = 'MPL-2'
  spec.authors = 'Hendrik Beskow'
  spec.add_test_dependency 'mruby-set'
  spec.add_test_dependency 'mruby-time'
  spec.add_test_dependency 'mruby-bigint'
  spec.add_test_dependency 'mruby-struct'
  spec.add_dependency 'mruby-errno'

  if spec.for_windows?
    spec.cxx_flags << '/std=c++17'
  else
    spec.cxx.flags << '-std=c++17'
  end
end
