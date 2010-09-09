require "swfmill_ext"

module Swfmill
  def self.swf2xml(string_or_io, option={})
    if string_or_io.respond_to? :read
      Swfmill.swf2xml(string_or_io.read, option)
    else
      encoding = option[:e] || option[:encoding] || 'UTF-8'
      Swfmill.to_xml(string_or_io.to_s, encoding.to_s)
    end
  end

  def self.xml2swf(string_or_io, option={})
    if string_or_io.respond_to? :read
      xml2swf(string_or_io.read, option)
    else
      encoding = option[:e] || option[:encoding] || 'UTF-8'
      to_swf(string_or_io.to_s, encoding.to_s)
    end
  end
end
