require "mkmf"

dir_config("freetype")
$FREETYPE_CFLAGS = "-I" + File.join(arg_config("--with-freetype-include", arg_config("--with-freetype-dir", "/usr/include")), "freetype2")
$FREETYPE_LIBS   = "-L" + arg_config("--with-freetype-lib", "/usr/lib") + " -lfreetype"
$CFLAGS  += " " + $FREETYPE_CFLAGS
have_library("freetype")
have_header("ft2build.h")

dir_config("libxml2")
$XML_CFLAGS = "-I" + File.join(arg_config("--with-libxml2-include", arg_config("--with-libxml2-dir", "/usr/include")), "libxml2")
$XML_LIBS   = "-L" + arg_config("--with-libxml2-lib", "/usr/lib") + " -lxml2"
$CFLAGS  += " " + $XML_CFLAGS
have_library("xml2")
have_header("libxml/tree.h")
have_header("libxml/uri.h")
have_header("libxml/xpathInternals.h")

dir_config("libxslt")
$XSLT_CFLAGS = "-I" + File.join(arg_config("--with-libxslt-include", arg_config("--with-libxslt-dir", "/usr/include")), "libxml2")
$XSLT_LIBS   = "-L" + arg_config("--with-libxslt-lib", "/usr/lib") + " -lexslt -lxslt"
$CFLAGS  += " " + $XSLT_CFLAGS
have_library("exslt")
have_library("xslt")
have_header("libexslt/exslt.h")
have_header("libxslt/extensions.h")
have_header("libxslt/transform.h")
have_header("libxslt/variables.h")
have_header("libxslt/xsltutils.h")

dir_config("libpng")
$PNG_CFLAGS = "-I" + File.join(arg_config("--with-libpng-include", arg_config("--with-libpng-dir", "/usr/include")))
$PNG_LIBS   = "-L" + arg_config("--with-libpng-lib", "/usr/lib") + " -lpng"
$CFLAGS  += " " + $PNG_CFLAGS
have_library("png")
have_header("png.h")

dir_config("zlib")
have_header("zlib.h")

have_library("stdc++")

$CFLAGS  += " -I./swfmill/src -I./swfmill/src/swft -I./swfmill/src/xslt"
$LDFLAGS += " -L./swfmill/src/swft/.libs -lswft -L./swfmill/src/xslt/.libs -lswfmillxslt"

create_makefile("swfmill_ext")

Dir.chdir(File.dirname(__FILE__) + "/swfmill") do
  env  = %Q|FREETYPE_CFLAGS=\"#{$FREETYPE_CFLAGS}\" FREETYPE_LIBS=\"#{$FREETYPE_LIBS}\" |
  env += %Q|XML_CFLAGS=\"#{$XML_CFLAGS}\" XML_LIBS=\"#{$XML_LIBS}\" |
  env += %Q|XSLT_CFLAGS=\"#{$XSLT_CFLAGS}\" XSLT_LIBS=\"#{$XSLT_LIBS}\" |
  env += %Q|PNG_CFLAGS=\"#{$PNG_CFLAGS}\" PNG_LIBS=\"#{$PNG_LIBS}\" |
  cmd = env + "./configure -C --disable-dependency-tracking"
  raise "cannot exec configure" unless system(cmd)
  cmd = "make"
  raise "cannot exec make" unless system(cmd)
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
               "swfmill-gSWFWriter.o",]
File.open(File.dirname(__FILE__) + "/Makefile", "ab") do |f|
  f.puts "OBJS += " + append_objs.map{ |e| objs_prefix + e }.join(" ")
end
