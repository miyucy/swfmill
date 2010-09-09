require 'rubygems'
require 'rake'

begin
  require 'jeweler'
  Jeweler::Tasks.new do |gem|
    gem.name = "swfmill"
    gem.summary = %Q{swfmill bindings for Ruby}
    gem.description = %Q{swfmill bindings for Ruby}
    gem.email = "miyucy@gmail.com"
    gem.homepage = "http://github.com/miyucy/swfmill"
    gem.authors = ["miyucy"]
    gem.add_development_dependency "rspec", ">= 1.2.9"
    # gem is a Gem::Specification... see http://www.rubygems.org/read/chapter/20 for additional settings
    gem.files << File.readlines(".swfmill").map{ |fn| fn.chomp }
  end
  Jeweler::GemcutterTasks.new
rescue LoadError
  puts "Jeweler (or a dependency) not available. Install it with: gem install jeweler"
end

require 'spec/rake/spectask'
Spec::Rake::SpecTask.new(:spec) do |spec|
  spec.libs << 'lib' << 'spec'
  spec.spec_files = FileList['spec/**/*_spec.rb']
end

Spec::Rake::SpecTask.new(:rcov) do |spec|
  spec.libs << 'lib' << 'spec'
  spec.pattern = 'spec/**/*_spec.rb'
  spec.rcov = true
end

task :spec => [:check_dependencies, :compile]

task :default => :spec

require 'rake/rdoctask'
Rake::RDocTask.new do |rdoc|
  version = File.exist?('VERSION') ? File.read('VERSION') : ""

  rdoc.rdoc_dir = 'rdoc'
  rdoc.title = "swfmill #{version}"
  rdoc.rdoc_files.include('README*')
  rdoc.rdoc_files.include('lib/**/*.rb')
end

task :compile => ['lib/swfmill_ext.so']
file 'lib/swfmill_ext.so' => FileList['ext/extconf.rb', 'ext/Makefile', 'ext/*.c', 'ext/*.cc', 'ext/*.h'] do
  Dir.chdir('ext') do
    sh 'make'
  end
  sh 'mv ext/swfmill_ext.so lib/swfmill_ext.so'
end
file 'ext/Makefile' => 'ext/extconf.rb' do
  Dir.chdir('ext') do
    ruby 'extconf.rb'
  end
end
