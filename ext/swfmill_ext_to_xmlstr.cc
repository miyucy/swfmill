#include "ruby.h"
#include "SWF.h"
#include "SWFReader.h"

extern VALUE rb_eSwfmill_Error;
extern VALUE rb_eSwfmill_EOFError;

/*
  call-seq:
    Swfmill.to_xmlstr(str [, params]) -> String

  +params+
    {
      :version => Fixnum,
      :compressed => Boolean,
      :encoding => String,
    }
 */
VALUE swfmill_ext_to_xmlstr(int argc, VALUE *argv, VALUE self)
{
    VALUE arg_str, arg_param;
    int arg_num = rb_scan_args(argc, argv, "11", &arg_str, &arg_param);

    unsigned char version = 7;
    bool       compressed = false;
    char*        encoding = NULL;

    Check_Type(arg_str, T_STRING);
    if(arg_num == 2)
    {
        Check_Type(arg_param, T_HASH);

        VALUE ver = rb_hash_aref(arg_param, ID2SYM(rb_intern("version")));
        if(!NIL_P(ver))
        {
            version = (unsigned char)NUM2INT(ver);
        }

        VALUE cmp = rb_hash_aref(arg_param, ID2SYM(rb_intern("compressed")));
        if(!NIL_P(cmp))
        {
            compressed = cmp == Qtrue;
        }

        VALUE enc = rb_hash_aref(arg_param, ID2SYM(rb_intern("encoding")));
        if(!NIL_P(enc))
        {
            encoding = StringValuePtr(enc);
        }
    }

    unsigned char* data = (unsigned char*)StringValuePtr(arg_str);
    size_t         size = RSTRING_LEN(arg_str);

    SWF::Header header;
    {
        SWF::Context context;
        SWF::Reader reader(data, size);
        context.swfVersion = version;
        header.parse(&reader, size, &context);
        switch(reader.getError())
        {
        case SWFR_ERROR:
            rb_raise(rb_eSwfmill_Error, "unknown error while reading SWF");
            break;
        case SWFR_EOF:
            rb_raise(rb_eSwfmill_EOFError, "reached EOF while reading SWF");
            break;
        }
    }

    VALUE xml = Qnil;
    {
        SWF::Context context;
        xmlDocPtr  doc  = xmlNewDoc((const xmlChar*)"1.0");
        xmlNodePtr root = doc->xmlRootNode = xmlNewDocNode(doc, NULL, (const xmlChar *)"swf", NULL);
        char       tmp[4];

        snprintf(tmp, sizeof(tmp), "%i", version);
        xmlSetProp(root, (const xmlChar*)"version", (const xmlChar*)tmp);

        snprintf(tmp, sizeof(tmp), "%i", compressed?1:0);
        xmlSetProp(root, (const xmlChar*)"compressed", (const xmlChar*)tmp);

        context.swfVersion = version;
        if(encoding != NULL)
        {
            context.convertEncoding = true;
            context.swf_encoding = encoding;
        }

        header.writeXML(root, &context);

        char* xml_data = NULL;
        int   xml_size;
        xmlDocDumpMemoryEnc(doc, (xmlChar **)&xml_data, &xml_size, "UTF-8");

        if(xml_size > 0)
        {
            xml = rb_str_new(xml_data, xml_size);
        }

        if(xml_data != NULL)
        {
            xmlFree(xml_data);
        }

        xmlFreeDoc(doc);
    }

    return xml;
}
