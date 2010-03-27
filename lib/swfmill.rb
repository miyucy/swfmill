require "swfmill_ext"
require "zlib"

module Swfmill
  def self.parse(str, opt=nil)
    str = str.to_s
    opt = {}

    signature = str[0, 3]
    raise Error unless signature == "CWS" || signature == "FWS"

    version = str[3, 1].unpack("C").first

    data = str[8 .. -1]
    data = Zlib::Inflate.inflate(data) if signature == "CWS"

    to_xmlstr(data, opt.update(:version => version, :compressed => (signature == "CWS")))
  end

  def self.load_file(fn, opt=nil)
    open(fn, "rb"){ |f| load_stream(f, opt) }
  end

  def self.load_stream(st, opt=nil)
    parse(st.read)
  end

  def self.publish(xmlstr, opt={})
    to_swf(xmlstr, opt)
  end
end
