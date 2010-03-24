$LOAD_PATH.unshift(File.dirname(__FILE__))
$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))
require 'swfmill'
require 'spec'
require 'spec/autorun'

Spec::Runner.configure do |config|
  
end

def dummy_data
  fn =  File.expand_path(File.dirname(__FILE__) + '/data/swfmill-banner1.swf')
  File.open(fn, 'rb'){ |f| f.read  }
end
