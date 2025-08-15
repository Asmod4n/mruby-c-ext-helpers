MRuby::Gem::Specification.new('mruby-c-ext-helpers') do |spec|
  spec.license = 'MPL-2'
  spec.authors = 'Hendrik Beskow'
  spec.add_test_dependency 'mruby-set'
  spec.add_test_dependency 'mruby-time'
  spec.add_dependency 'mruby-errno'

  spec.cxx.flags << '-std=c++17'
end
