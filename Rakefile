require 'rake'
require 'fileutils'

MRUBY_CONFIG_PATH = File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :mruby do
  unless File.directory?('mruby')
    sh "git clone --depth=1 https://github.com/mruby/mruby.git"
  end
end

desc "compile binary"
task :compile => :mruby do
  sh({"MRUBY_CONFIG" => MRUBY_CONFIG_PATH}, "rake all", chdir: "mruby")
end

desc "test"
task :test => :mruby do
  sh({"MRUBY_CONFIG" => MRUBY_CONFIG_PATH}, "rake all test", chdir: "mruby")
end

desc "cleanup"
task :clean do
  sh({"MRUBY_CONFIG" => MRUBY_CONFIG_PATH}, "rake deep_clean", chdir: "mruby")
end

task :default => :test
