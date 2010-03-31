#include <zlib.h>
#include "ruby.h"
#include "SWF.h"
#include "SWFWriter.h"
#include "deflate.h"

extern VALUE rb_eSwfmill_Error;
extern VALUE rb_eSwfmill_EOFError;

/*
  call-seq:
    Swfmill.to_swf(str [, params]) -> Array

  +params+
    {
      :compress => Boolean,
      :level => Symbol,
    }

  :compress
    true
    false
    nil

  :level
    :best
    :speed
    :none
 */
VALUE swfmill_ext_to_swf(int argc, VALUE *argv, VALUE self)
{
    VALUE arg_str, arg_param;
    int arg_num = rb_scan_args(argc, argv, "11", &arg_str, &arg_param);

    unsigned char version = 7;
    bool       compressed = false;
    int          compress = 0;
    int    compress_level = Z_BEST_COMPRESSION;
    char*        encoding = NULL;

    Check_Type(arg_str, T_STRING);
    if(arg_num == 2)
    {
        Check_Type(arg_param, T_HASH);

        VALUE lvl = rb_hash_aref(arg_param, ID2SYM(rb_intern("level")));
        if(!NIL_P(lvl))
        {
            if(lvl == ID2SYM(rb_intern("speed")))
            {
                compress_level = Z_BEST_SPEED;
            }
            if(lvl == ID2SYM(rb_intern("none")))
            {
                compress_level = Z_NO_COMPRESSION;
            }
        }

        VALUE cmp = rb_hash_aref(arg_param, ID2SYM(rb_intern("compressed")));
        if(!NIL_P(cmp))
        {
            compress = cmp == Qtrue ? 1 : 2;
        }

        VALUE enc = rb_hash_aref(arg_param, ID2SYM(rb_intern("encoding")));
        if(!NIL_P(enc))
        {
            encoding = StringValuePtr(enc);
        }
    }

    char*      data = StringValuePtr(arg_str);
    int        size = RSTRING_LEN(arg_str);
    xmlDocPtr  doc  = xmlParseMemory(data, size);

    if(doc == NULL)
    {
        rb_raise(rb_eSwfmill_Error, "XML parser error");
    }

    xmlNodePtr root = doc->xmlRootNode;
    if(strcmp((const char*)root->name, "swf") != 0)
    {
        xmlFreeDoc(doc);
        rb_raise(rb_eSwfmill_Error, "doesn't seem to be a swfml file");
    }

    xmlNodePtr headerNode = root->children;
    while((headerNode != NULL) &&
          (headerNode->name == NULL || strcmp((const char*)headerNode->name, "Header") != 0))
    {
        headerNode = headerNode->next;
    }
    if(headerNode == NULL)
    {
        xmlFreeDoc(doc);
        rb_raise(rb_eSwfmill_Error, "swfml file is empty");
    }

    {
        xmlChar *tmp;
        tmp = xmlGetProp(root, (const xmlChar*)"version");
        if(tmp != NULL)
        {
            int i;
            sscanf((char*)tmp, "%i", &i);
            version = i;
            xmlFree(tmp);
        }

        tmp = xmlGetProp(root, (const xmlChar*)"compressed");
        if(tmp != NULL)
        {
            int i;
            sscanf((char*)tmp, "%i", &i);
            compressed = i > 0;
            xmlFree(tmp);
        }
    }
    if(compress > 0)
    {
        compressed = compress == 1;
    }

    SWF::Header header;
    size_t swf_size;
    {
        SWF::Context context;
        context.swfVersion = version;
        if(encoding != NULL)
        {
            context.convertEncoding = true;
            context.swf_encoding = encoding;
        }
        header.parseXML(headerNode, &context);
        swf_size = header.getSize(&context, 0) / 8;
    }

    char swf_head[8+1];
    {
        swf_size += 8;
        snprintf(swf_head, 8, "%s%c%c%c%c%c",
                 compressed ? "CWS" : "FWS",
                 version,
                 ((swf_size>> 0) & 0xFF),
                 ((swf_size>> 8) & 0xFF),
                 ((swf_size>>16) & 0xFF),
                 ((swf_size>>24) & 0xFF));
        swf_size -= 8;
    }

    unsigned char* swf_buff = (unsigned char*)ruby_xmalloc(swf_size);
    {
        SWF::Context context;
        SWF::Writer writer(swf_buff, swf_size);
        context.swfVersion = version;
        header.write(&writer, &context);
        switch(writer.getError())
        {
        case SWFW_ERROR:
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "unknown write error");
            break;
        case SWFW_FULL:
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "write buffer full");
            break;
        }
    }

    if(compressed)
    {
        Deflate deflate(swf_buff, swf_size, compress_level);
        if(!deflate.compress())
        {
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "Error compressing SWF: %s", deflate.error());
        }
        ruby_xfree(swf_buff);
        swf_size = deflate.size();
        swf_buff = (unsigned char*)ruby_xmalloc(swf_size);
        memcpy(swf_buff, deflate.data(), swf_size);
    }

    VALUE swf;
    swf = rb_str_new(swf_head, 8);
    rb_str_cat(swf, (const char*)swf_buff, swf_size);

    ruby_xfree(swf_buff);
    xmlFreeDoc(doc);
    return swf;
}
