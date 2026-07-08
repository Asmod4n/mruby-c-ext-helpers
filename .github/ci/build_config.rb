MRuby::Build.new do |conf|
  case ENV['MRUBY_TOOLCHAIN']
  when 'clang'     then toolchain :clang
  when 'visualcpp' then toolchain :visualcpp
  else                  toolchain :gcc
  end

  if ENV['MRUBY_SANITIZE'] == '1'
    conf.enable_debug
    conf.enable_sanitizer 'address,undefined'
    conf.cc.flags << '-fno-omit-frame-pointer'
  end

  conf.enable_test
  conf.gembox 'default'
  conf.gem File.expand_path('../..', __dir__)
end
