MRuby::Build.new do |conf|
  toolchain :gcc
  def for_windows?
    ('A'..'Z').to_a.any? { |vol| Dir.exist?("#{vol}:") }
  end

  unless for_windows?
    conf.enable_sanitizer "address,undefined,leak"
    conf.linker.flags_before_libraries << '-static-libasan'
  end
  conf.cc.flags << '-fno-omit-frame-pointer' << '-g' << '-ggdb'
  enable_debug
  conf.enable_debug
  conf.enable_test
  conf.gembox 'full-core'
  conf.gem File.expand_path(File.dirname(__FILE__))
end
