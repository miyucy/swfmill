# -*- coding: utf-8 -*-
require File.expand_path(File.dirname(__FILE__) + '/spec_helper')
require "tempfile"

context Swfmill, ".load_file" do
  it "openできなければ例外が発生すること" do
    lambda{ Swfmill.load_file(nil) }.should raise_exception
  end

  it "load_streamをコールすること" do
    Swfmill.should_receive(:load_stream)
    Swfmill.load_file(Tempfile.new(File.basename(__FILE__)).path)
  end
end

context Swfmill, ".load_stream" do
  it "readできなければ例外が発生すること" do
    lambda{ Swfmill.load_stream(Object.new) }.should raise_exception
  end

  it "parseをコールすること" do
    Swfmill.should_receive(:parse).with(true)
    Swfmill.load_stream(stub("readable", :read => true))
  end
end

context Swfmill, ".parse" do
  it "引数をto_sすること" do
    Swfmill.should_receive(:to_xmlstr)
    Swfmill.parse(mock("to_s", :to_s => dummy_data))
  end

  it "不正なデータを受け取ったらSwfmill::Errorを返すこと" do
    lambda{ Swfmill.parse("invalid_swf_data") }.should raise_exception Swfmill::Error
  end

  it "FWSで始まっていないデータを受け取ったらSwfmill::Errorを返すこと" do
    lambda{ Swfmill.parse("fws") }.should raise_exception Swfmill::Error
    lambda{ Swfmill.parse("fWS") }.should raise_exception Swfmill::Error
    lambda{ Swfmill.parse("FWS") }.should_not raise_exception Swfmill::Error
  end

  it "CWSで始まっていないデータを受け取ったらSwfmill::Errorを返すこと" do
    lambda{ Swfmill.parse("cws") }.should raise_exception Swfmill::Error
    lambda{ Swfmill.parse("cWS") }.should raise_exception Swfmill::Error
    lambda{ Swfmill.parse("CWS") }.should_not raise_exception Swfmill::Error
  end
end

context Swfmill, ".parseにCWSを渡した場合" do
  it "Zlib::Inflate.inflateがコールされること" do
    Zlib::Inflate.should_receive(:inflate)

    Swfmill.should_receive(:to_xmlstr)
    Swfmill.parse("CWS")
  end

  it "Zlib::Inflate.inflateに8バイト以降のデータが渡されること" do
    Zlib::Inflate.should_receive(:inflate).with("12345").and_return("12345")

    Swfmill.should_receive(:to_xmlstr).with("12345", anything)
    Swfmill.parse("CWSxxxxx12345")
  end

  it "第2引数に{ :version => 9, :compressed => false }を受け取ること" do
    Swfmill.should_receive(:to_xmlstr).with("abc", { :version => 9, :compressed => false })
    Swfmill.parse("FWS\x09xxxxabc")
  end

  it "第2引数に{ :version => 1, :compressed => true }を受け取ること" do
    Swfmill.should_receive(:to_xmlstr).with("abc", { :version => 1, :compressed => true })
    Swfmill.parse("CWS\x01xxxx" + Zlib::Deflate.deflate("abc"))
  end
end

context Swfmill, ".to_xmlstr" do
  it { lambda{ Swfmill.to_xmlstr }.should raise_exception }
  it { lambda{ Swfmill.to_xmlstr(:non_string_object) }.should raise_exception }
  it { lambda{ Swfmill.to_xmlstr("", :non_hash_object) }.should raise_exception }

  it "XML文字列を返すこと" do
    Swfmill.parse(dummy_data).should include '<?xml version="1.0"?>'
  end
end

context Swfmill, ".to_swf" do
  it do
    lambda { Swfmill.to_swf("") }.should raise_exception
  end

  it do
    xml = <<XML
<?xml version="1.0"?>
<fla version="7" compressed="0">
</fla>
XML
    lambda { Swfmill.to_swf(xml) }.should raise_exception
  end

  it do
    xml = <<XML
<?xml version="1.0"?>
<swf version="7" compressed="0">
</swf>
XML
    lambda { Swfmill.to_swf(xml) }.should raise_exception
  end

  it do
    xml = <<XML
<?xml version="1.0"?>
<swf version="7" compressed="0">
  <Header framerate="0">
  </Header>
</swf>
XML
    lambda { Swfmill.to_swf(xml) }.should_not raise_exception
  end
end

context Swfmill, ".publish" do
  it "Swfmill.parseしたデータを元に戻せること" do
    Swfmill.publish(Swfmill.parse(dummy_data)).should == dummy_data
  end
end
