#include <zlib.h>
#include "ruby.h"
#include "SWF.h"
#include "SWFReader.h"
#include "SWFWriter.h"

static VALUE rb_mSwfmill;
static VALUE rb_eSwfmill_Error;
static VALUE rb_eSwfmill_EOFError;

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
    VALUE arg_str, arg_param;
    int arg_num = rb_scan_args(argc, argv, "11", &arg_str, &arg_param);

    unsigned char version = 7;
    bool       compressed = false;

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
    }

    unsigned char* data = (unsigned char*)StringValuePtr(arg_str);
    size_t         size = RSTRING_LEN(arg_str);

    SWF::Header header;
    {
        SWF::Context context;
        SWF::Reader reader(data, size);
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
        header.writeXML(root, &context);

        char* xml_data = NULL;
        int   xml_size;
        xmlDocDumpFormatMemory(doc, (xmlChar **)&xml_data, &xml_size, 1);

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
        unsigned char temp[BUFSIZ];
        z_stream stream={Z_NULL};
        int status, count;

        unsigned char* out_buff = (unsigned char*)ruby_xmalloc(swf_size);
        int out_offset = 0, out_size = swf_size;

        stream.next_in   = swf_buff;
        stream.avail_in  = swf_size;
        stream.next_out  = temp;
        stream.avail_out = BUFSIZ;

        status = deflateInit(&stream, compress_level);
        if(status != Z_OK)
        {
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "Error compressing SWF: %s", stream.msg);
        }

        for(;;)
        {
            if(stream.avail_in == 0)
            {
                break;
            }

            status = deflate(&stream, Z_NO_FLUSH);
            if(status != Z_OK)
            {
                ruby_xfree(swf_buff);
                xmlFreeDoc(doc);
                rb_raise(rb_eSwfmill_Error, "Error compressing SWF: %s", stream.msg);
            }

            count = BUFSIZ - stream.avail_out;
            if(count > 0)
            {
                if(out_offset + count >= out_size)
                {
                    int tmp_size = out_size + out_size / 2;
                    unsigned char* tmp_buff = (unsigned char*)ruby_xmalloc(tmp_size);
                    memcpy(tmp_buff, out_buff, out_size);
                    ruby_xfree(out_buff);
                    out_buff = tmp_buff;
                    out_size = tmp_size;
                }
                memcpy(out_buff + out_offset, temp, count);
                out_offset += count;
            }

            stream.next_out  = temp;
            stream.avail_out = BUFSIZ;
        }

        do
        {
            status = deflate(&stream, Z_FINISH);

            count = BUFSIZ - stream.avail_out;
            if(count > 0)
            {
                if(out_offset + count >= out_size)
                {
                    int tmp_size = out_size + out_size / 2;
                    unsigned char* tmp_buff = (unsigned char*)ruby_xmalloc(tmp_size);
                    memcpy(tmp_buff, out_buff, out_size);
                    ruby_xfree(out_buff);
                    out_buff = tmp_buff;
                    out_size = tmp_size;
                }
                memcpy(out_buff + out_offset, temp, count);
                out_offset += count;
            }

            stream.next_out  = temp;
            stream.avail_out = BUFSIZ;
        } while(status == Z_OK);

        if(status != Z_STREAM_END)
        {
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "Error compressing SWF: %s", stream.msg);
        }

        status = deflateEnd(&stream);
        if(status != Z_OK)
        {
            ruby_xfree(swf_buff);
            xmlFreeDoc(doc);
            rb_raise(rb_eSwfmill_Error, "Error compressing SWF: %s", stream.msg);
        }

        ruby_xfree(swf_buff);
        swf_size = out_offset;
        swf_buff = (unsigned char*)ruby_xmalloc(swf_size);
        memcpy(swf_buff, out_buff, swf_size);
        ruby_xfree(out_buff);
    }

    VALUE swf;
    swf = rb_str_new(swf_head, 8);
    rb_str_cat(swf, (const char*)swf_buff, swf_size);

    ruby_xfree(swf_buff);
    xmlFreeDoc(doc);
    return swf;
}

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
