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

$CFLAGS  += " -I./swfmill/src -I./swfmill/src/swft -I./swfmill/src/xslt"

Dir.chdir("swfmill") do
  cmd = "sh autogen.sh"
  raise "error: #{cmd}" unless system(cmd)
  cmd = <<CMD
sh configure --disable-dependency-tracking \
FREETYPE_CFLAGS="#{$FREETYPE_CFLAGS}" FREETYPE_LIBS="#{$FREETYPE_LIBS}" \
XML_CFLAGS="#{$XML_CFLAGS}" XML_LIBS="#{$XML_LIBS}" \
XSLT_CFLAGS="#{$XSLT_CFLAGS}" XSLT_LIBS="#{$XSLT_LIBS}" \
PNG_CFLAGS="#{$PNG_CFLAGS}" PNG_LIBS="#{$PNG_LIBS}"
CMD
  raise "error: #{cmd}" unless system(cmd)
  cmd = "make"
  raise "error: #{cmd}" unless system(cmd)
end

objs_prefix = "swfmill/src/"
append_objs = ["swfmill-Geom.o",
               "swfmill-SWFAction.o",
               "swfmill-SWFFile.o",
               "swfmill-SWFFilter.o",
               "swfmill-SWFGlyphList.o",
               "swfmill-SWFItem.o",
               "swfmill-SWFReader.o",
               "swfmill-SWFShapeItem.o",
               "swfmill-SWFShapeMaker.o",
               "swfmill-SWFTag.o",
               "swfmill-SWFTrait.o",
               "swfmill-SWFWriter.o",
               "swfmill-base64.o",
               "swfmill-gSWFBasics.o",
               "swfmill-gSWFDumper.o",
               "swfmill-gSWFParseXML.o",
               "swfmill-gSWFParser.o",
               "swfmill-gSWFSize.o",
               "swfmill-gSWFWriteXML.o",
               "swfmill-gSWFWriter.o",
               "swft/libswft_la-Parser.o",
               "swft/libswft_la-SVGAttributeParser.o",
               "swft/libswft_la-SVGColor.o",
               "swft/libswft_la-SVGGradient.o",
               "swft/libswft_la-SVGPathParser.o",
               "swft/libswft_la-SVGPointsParser.o",
               "swft/libswft_la-SVGStyle.o",
               "swft/libswft_la-SVGTransformParser.o",
               "swft/libswft_la-readpng.o",
               "swft/libswft_la-swft.o",
               "swft/libswft_la-swft_document.o",
               "swft/libswft_la-swft_import.o",
               "swft/libswft_la-swft_import_binary.o",
               "swft/libswft_la-swft_import_jpeg.o",
               "swft/libswft_la-swft_import_mp3.o",
               "swft/libswft_la-swft_import_png.o",
               "swft/libswft_la-swft_import_ttf.o",
               "swft/libswft_la-swft_import_wav.o",
               "swft/libswft_la-swft_path.o",
               "xslt/libswfmillxslt_la-simple.o",]

create_makefile("swfmill_ext")
File.open(File.dirname("__FILE__") + "/Makefile", "ab") do |f|
  f.puts "OBJS += #{append_objs.map{ |e| objs_prefix + e }.join(" ")}"
end
