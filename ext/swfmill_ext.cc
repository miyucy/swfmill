#include "ruby.h"
#include "swfmill_ext_to_xmlstr.h"
#include "swfmill_ext_to_swf.h"

static VALUE rb_mSwfmill;
VALUE rb_eSwfmill_Error;
VALUE rb_eSwfmill_EOFError;

extern "C" {
    void Init_swfmill_ext(void)
    {
        rb_mSwfmill = rb_define_module("Swfmill");
        rb_define_singleton_method(rb_mSwfmill, "to_xmlstr", (VALUE(*)(...))swfmill_ext_to_xmlstr, -1);
        rb_define_singleton_method(rb_mSwfmill, "to_swf", (VALUE(*)(...))swfmill_ext_to_swf, -1);

        rb_eSwfmill_Error = rb_define_class_under(rb_mSwfmill, "Error", rb_eStandardError);
        rb_eSwfmill_EOFError = rb_define_class_under(rb_mSwfmill, "EOFError", rb_eStandardError);
    }
}
