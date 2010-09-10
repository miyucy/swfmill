#include "ruby.h"
#include "swfmill_ext_to_xml.h"
#include "swfmill_ext_to_swf.h"
#include <libxml/xmlmemory.h>

VALUE rb_mSwfmill;
VALUE rb_eSwfmill_Error;
VALUE rb_eSwfmill_EOFError;

extern "C" {

static void
init_xml_memory()
{
    xmlMemSetup((xmlFreeFunc)ruby_xfree,
                (xmlMallocFunc)ruby_xmalloc,
                (xmlReallocFunc)ruby_xrealloc,
                strdup);
}

void Init_swfmill_ext(void)
{
    init_xml_memory();

    rb_mSwfmill = rb_define_module("Swfmill");
    rb_eSwfmill_Error = rb_define_class_under(rb_mSwfmill, "Error", rb_eStandardError);
    rb_eSwfmill_EOFError = rb_define_class_under(rb_mSwfmill, "EOFError", rb_eStandardError);

    rb_define_singleton_method(rb_mSwfmill, "to_xml", (VALUE (*)(...))swfmill_ext_to_xml, 2);
    rb_define_singleton_method(rb_mSwfmill, "to_swf", (VALUE (*)(...))swfmill_ext_to_swf, 2);
}

}
