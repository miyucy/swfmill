require "mkmf"

def arg_include_exist?(target)
  !!(arg_config("--with-#{target}-include") || arg_config("--with-#{target}-dir"))
end

def arg_include(target)
  arg_config("--with-#{target}-include", arg_config("--with-#{target}-dir", "/usr") + "/include")
end

def arg_lib_exist?(target)
  !!(arg_config("--with-#{target}-lib") || arg_config("--with-#{target}-dir"))
end

def arg_lib(target)
  arg_config("--with-#{target}-lib", arg_config("--with-#{target}-dir", "/usr") + "/lib")
end

dir_config("freetype")
if arg_include_exist?("freetype")
  $FREETYPE_CFLAGS = "-I#{arg_include("freetype") + "/freetype2"}"
  $CFLAGS += " #{$FREETYPE_CFLAGS}"
end
if arg_lib_exist?("freetype")
  $FREETYPE_LIBS = "-L#{arg_lib("freetype")} -lfreetype"
  $LDFLAGS += " -L#{arg_lib("freetype")}"
  $libs += " -lfreetype"
end
unless $FREETYPE_CFLAGS and $FREETYPE_LIBS
  $FREETYPE_CFLAGS, ldflags, libs = pkg_config("freetype2")
  $FREETYPE_LIBS = "#{ldflags} #{libs}"
end
have_library("freetype")
have_header("ft2build.h")

dir_config("libxml2")
if arg_include_exist?("libxml2")
  $XML_CFLAGS = "-I#{arg_include("libxml2") + "/libxml2"}"
  $CFLAGS += " #{$XML_CFLAGS}"
end
if arg_lib_exist?("libxml2")
  $XML_LIBS = "-L#{arg_lib("libxml2")} -lxml2"
  $LDFLAGS += " -L#{arg_lib("libxml2")}"
  $libs += " -lxml2"
end
unless $XML_CFLAGS and $XML_LIBS
  $XML_CFLAGS, ldflags, libs = pkg_config("libxml-2.0")
  $XML_LIBS = "#{ldflags} #{libs}"
end
have_library("xml2")
have_header("libxml/tree.h")
have_header("libxml/uri.h")
have_header("libxml/xpathInternals.h")

dir_config("libexslt")
if arg_include_exist?("libexslt")
  $XSLT_CFLAGS = "-I#{arg_include("libexslt") + "/libxml2"}"
  $CFLAGS += " #{XSLT_CFLAGS}"
end
if arg_lib_exist?("libexslt")
  $XSLT_LIBS = "-L#{arg_lib("libexslt")} -lexslt -lxslt -lxml2"
  $LDFLAGS += " -L#{arg_lib("libexslt")}"
  $libs += " -lexslt -lxslt -lxml2"
end
unless $XSLT_CFLAGS and $XSLT_LIBS
  $XSLT_CFLAGS, ldflags, libs = pkg_config("libexslt")
  $XSLT_LIBS = "#{ldflags} #{libs}"
end
have_library("exslt")
have_library("xslt")
have_header("libexslt/exslt.h")
have_header("libxslt/extensions.h")
have_header("libxslt/transform.h")
have_header("libxslt/variables.h")
have_header("libxslt/xsltutils.h")

dir_config("libpng")
if arg_include_exist?("libpng")
  $PNG_CFLAGS = "-I#{arg_include("libpng")}"
  $CFLAGS += " #{$PNG_CFLAGS}"
end
if arg_lib_exist?("libpng")
  $PNG_LIBS = "-L#{arg_lib("libpng")} -lpng"
  $LDFLAGS += " -L#{arg_lib("libpng")}"
  $libs += " -lpng"
end
unless $PNG_CFLAGS and $PNG_LIBS
  $PNG_CFLAGS, ldflags, libs = pkg_config("libpng")
  $PNG_LIBS = "#{ldflags} #{libs}"
end
have_library("png")
have_header("png.h")

dir_config("zlib")
have_header("zlib.h")

have_func("iconv", "iconv.h") or have_library("iconv", "iconv", "iconv.h")

have_library("stdc++")

$INCFLAGS += " -I./swfmill/src -I./swfmill/src/swft -I./swfmill/src/xslt -I./libb64/include"

Dir.chdir("libb64") do
  cmd = "make"
  raise "error: #{cmd}" unless system(cmd)
end

Dir.chdir("swfmill/src") do
  cmd = "xsltproc codegen/mk.xsl codegen/source.xml"
  raise "error: #{cmd}" unless system(cmd)

  namespaces  = 'xmlns:swft="http://subsignal.org/swfml/swft" '
  namespaces << 'xmlns:str="http://exslt.org/strings" '
  namespaces << 'xmlns:math="http://exslt.org/math"'

  simple = %x"xsltproc xslt/assemble.xsl xslt/simple.xml"
  File.open("simple.cpp", "wb") do |f|
    f.puts '#include "xslt/xslt.h"'
    f.puts 'const char *xslt_simple = '
    f.puts simple.sub(/namespaces="hack"/, namespaces).gsub(/"/, '\"').gsub(/^.*$/){ |m| '"' + m + '\n"' }
    f.puts ';'
  end
end

create_makefile("swfmill_ext")

File.open(File.dirname("__FILE__") + "/Makefile", "ab") do |f|
  f.puts "OBJS += libb64/src/cdecode.o libb64/src/cencode.o"
end

swfmill_files = <<FILES.split.map{|file| file.chomp}
swfmill/src/gSWFBasics.cpp
swfmill/src/gSWFDumper.cpp
swfmill/src/gSWFParseXML.cpp
swfmill/src/gSWFParser.cpp
swfmill/src/gSWFSize.cpp
swfmill/src/gSWFWriteXML.cpp
swfmill/src/gSWFWriter.cpp
swfmill/src/Geom.cpp
swfmill/src/SWFAction.cpp
swfmill/src/SWFFile.cpp
swfmill/src/SWFFilter.cpp
swfmill/src/SWFGlyphList.cpp
swfmill/src/SWFItem.cpp
swfmill/src/SWFReader.cpp
swfmill/src/SWFShapeItem.cpp
swfmill/src/SWFShapeMaker.cpp
swfmill/src/SWFTag.cpp
swfmill/src/SWFTrait.cpp
swfmill/src/SWFWriter.cpp
swfmill/src/simple.cpp
swfmill/src/swft/Parser.cpp
swfmill/src/swft/SVGAttributeParser.cpp
swfmill/src/swft/SVGColor.cpp
swfmill/src/swft/SVGGradient.cpp
swfmill/src/swft/SVGPathParser.cpp
swfmill/src/swft/SVGPointsParser.cpp
swfmill/src/swft/SVGStyle.cpp
swfmill/src/swft/SVGTransformParser.cpp
swfmill/src/swft/readpng.c
swfmill/src/swft/swft.cpp
swfmill/src/swft/swft_document.cpp
swfmill/src/swft/swft_import.cpp
swfmill/src/swft/swft_import_binary.cpp
swfmill/src/swft/swft_import_jpeg.cpp
swfmill/src/swft/swft_import_mp3.cpp
swfmill/src/swft/swft_import_png.cpp
swfmill/src/swft/swft_import_ttf.cpp
swfmill/src/swft/swft_import_wav.cpp
swfmill/src/swft/swft_path.cpp
FILES

data = File.open(File.dirname("__FILE__") + "/Makefile", "rb"){ |f| f.read }
data.gsub!(/^OBJS = .*\n/) do |m|
  objs = swfmill_files.map{ |e| File.join(File.dirname(e), File.basename(e, ".*") + ".o") }
  m << "OBJS+= " << objs.join(" ") << "\n"
end
swfmill_files.each do |e|
  o = File.join(File.dirname(e), File.basename(e, ".*") + ".o")
  data << "#{o}: #{e}\n"
  if e =~ /cpp\Z/
    data << "\t$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<\n"
  else
    data << "\t$(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<\n"
  end
end
File.open(File.dirname("__FILE__") + "/Makefile", "wb") { |f| f.write data }
