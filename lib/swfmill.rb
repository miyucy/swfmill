require "swfmill_ext"
require "zlib"

module Swfmill
  def self.load(str)
    str = str.to_s

    signature = str[0, 3]
    raise Error unless signature == "CWS" || signature == "FWS"

    version = str[3, 1].unpack("C").first

    data = str[8 .. -1]
    data = Zlib::Inflate.inflate(data) if signature == "CWS"

    to_xmlstr(data, :version => version, :compressed => (signature == "CWS"))
  end

  def self.load_file(fn)
    open(fn, "rb"){ |f| load_stream(f) }
  end

  def self.load_stream(io)
    load(io.read)
  end
end
