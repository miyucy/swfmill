#include "stdio.h"
#include "ruby.h"
#include "SWF.h"
#include "SWFReader.h"

static VALUE rb_mSwfmill;

/*
  call-seq:
    Swfmill.to_xmlstr(str [, params]) -> String

  +params+
    {
      :version => Fixnum,
      :compressed => Boolean,
    }
 */
VALUE swfmill_ext_to_xmlstr(int argc, VALUE *argv, VALUE self)
{
    return Qnil;
}

extern "C" {
    void Init_swfmill_ext(void)
    {
        rb_mSwfmill = rb_define_module("Swfmill");
        rb_define_singleton_method(rb_mSwfmill, "to_xmlstr", (VALUE(*)(...))swfmill_ext_to_xmlstr, -1);
    }
}
