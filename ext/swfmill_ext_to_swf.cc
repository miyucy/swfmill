#include <zlib.h>
#include "ruby.h"
#include "SWF.h"
#include "SWFWriter.h"
#include "deflate.h"

extern VALUE rb_eSwfmill_Error;
extern VALUE rb_eSwfmill_EOFError;

#define SIGNATURE_SIZE 8
typedef struct {
    VALUE            string;
    VALUE          encoding;
    bool         compressed;
    SWF::Header     *header;
    SWF::Context   *context;
    size_t         datasize;
    unsigned char *databuff;
    int      compress_level;
    xmlDocPtr      document;
    xmlNodePtr    root_node;
    xmlNodePtr  header_node;
} swfmill_to_swf;

static void
sts_mark(swfmill_to_swf * const sts)
{
    if(sts != NULL)
    {
        rb_gc_mark(sts->string);
        rb_gc_mark(sts->encoding);
    }
}

static void
sts_free(swfmill_to_swf * const sts)
{
    if(sts != NULL)
    {
        if(sts->document != NULL)
        {
            xmlFreeDoc(sts->document);
            sts->document = NULL;
        }

        ruby_xfree(sts->databuff);
        sts->databuff = NULL;

        delete sts->header;
        sts->header = NULL;

        delete sts->context;
        sts->context = NULL;
    }
}

static bool
sts_parse_document(swfmill_to_swf * const sts)
{
    sts->document = xmlParseMemory(RSTRING_PTR(sts->string), RSTRING_LEN(sts->string));
    if(sts->document == NULL)
    {
        rb_raise(rb_eSwfmill_Error, "XML parser error");
        return false;
    }

    sts->root_node = sts->document->xmlRootNode;
    if(strcmp((const char*)sts->root_node->name, "swf") != 0)
    {
        rb_raise(rb_eSwfmill_Error, "doesn't seem to be a swfml file");
        return false;
    }

    return true;
}

static bool
sts_search_header_node(swfmill_to_swf * const sts)
{
    xmlNodePtr headerNode = sts->root_node->children;

    while((headerNode != NULL) &&
          (headerNode->name == NULL || strcmp((const char*)headerNode->name, "Header") != 0))
    {
        headerNode = headerNode->next;
    }

    if(headerNode == NULL)
    {
        rb_raise(rb_eSwfmill_Error, "swfml file is empty");
        return false;
    }

    sts->header_node = headerNode;

    return true;
}

static void
sts_get_version(swfmill_to_swf * const sts)
{
    xmlChar *tmp = xmlGetProp(sts->root_node, (const xmlChar*)"version");

    sts->context = new SWF::Context;
    sts->context->debugTrace = false;
    sts->context->quiet      = true;

    if(tmp != NULL)
    {
        sscanf((char*)tmp, "%i", &sts->context->swfVersion);
        xmlFree(tmp);
    }
}

static void
sts_get_compressed(swfmill_to_swf * const sts)
{
    xmlChar *tmp = xmlGetProp(sts->root_node, (const xmlChar*)"compressed");
    int compress;

    sts->compressed = false;
    if(tmp != NULL)
    {
        sscanf((char*)tmp, "%i", &compress);
        sts->compressed = compress > 0;
        xmlFree(tmp);
    }
}

static void
sts_parse_xml(swfmill_to_swf * const sts)
{
    sts->context->convertEncoding = true;
    sts->context->swf_encoding = StringValueCStr(sts->encoding);

    sts->header = new SWF::Header;
    sts->header->parseXML(sts->header_node, sts->context);

    sts->datasize = sts->header->getSize(sts->context, 0) / 8;
    sts->databuff = (unsigned char*)ruby_xmalloc(sts->datasize + SIGNATURE_SIZE);
}

static bool
sts_compress(swfmill_to_swf * const sts)
{
    if(!sts->compressed)
    {
        return true;
    }

    Deflate deflate(sts->databuff + SIGNATURE_SIZE, sts->datasize, sts->compress_level);

    if(!deflate.compress())
    {
        rb_raise(rb_eSwfmill_Error, "compressing SWF: %s", deflate.error());
        return false;
    }

    ruby_xfree(sts->databuff);
    sts->databuff = NULL;

    sts->datasize = deflate.size();
    sts->databuff = (unsigned char*)ruby_xmalloc(sts->datasize + SIGNATURE_SIZE);
    memcpy(sts->databuff + SIGNATURE_SIZE, deflate.data(), sts->datasize);

    return true;
}

static bool
sts_write(swfmill_to_swf * const sts)
{
    SWF::Writer writer(sts->databuff + SIGNATURE_SIZE, sts->datasize);
    size_t    datasize;

    sts->header->write(&writer, sts->context);
    switch(writer.getError())
    {
    case SWFW_ERROR:
        rb_raise(rb_eSwfmill_Error, "unknown write error");
        return false;
    case SWFW_FULL:
        rb_raise(rb_eSwfmill_Error, "write buffer full");
        return false;
    }

    if(!sts_compress(sts))
    {
        return false;
    }

    sts->databuff[0] = (unsigned char)(sts->compressed ? 'C' : 'F');
    sts->databuff[1] = (unsigned char)'W';
    sts->databuff[2] = (unsigned char)'S';
    sts->databuff[3] = (unsigned char)sts->context->swfVersion;

    datasize = sts->datasize;
    sts->databuff[4] = (unsigned char)((datasize >>  0) & 0xFF);
    sts->databuff[5] = (unsigned char)((datasize >>  8) & 0xFF);
    sts->databuff[6] = (unsigned char)((datasize >> 16) & 0xFF);
    sts->databuff[7] = (unsigned char)((datasize >> 24) & 0xFF);

    return true;
}

static bool
sts_parse(swfmill_to_swf * const sts)
{
    if(!sts_parse_document(sts))
    {
        return false;
    }
    if(!sts_search_header_node(sts))
    {
        return false;
    }
    sts_get_version(sts);
    sts_get_compressed(sts);
    sts_parse_xml(sts);
    if(!sts_write(sts))
    {
        return false;
    }
    return true;
}

static VALUE
sts_to_string(swfmill_to_swf * const sts)
{
    VALUE str = rb_str_new((const char*)sts->databuff, sts->datasize + SIGNATURE_SIZE);

    ruby_xfree(sts->databuff);
    sts->databuff = NULL;

    return str;
}

/*
  call-seq:
    Swfmill.to_swf(str, encoding) -> String
 */
VALUE swfmill_ext_to_swf(VALUE self, VALUE string, VALUE encoding)
{
    swfmill_to_swf *sts;

    Check_Type(string, T_STRING);
    Check_Type(encoding, T_STRING);

    Data_Make_Struct(rb_cData, swfmill_to_swf, sts_mark, sts_free, sts);

    sts->string         = string;
    sts->encoding       = encoding;
    sts->compress_level = Z_BEST_COMPRESSION;

    if(sts_parse(sts))
    {
        return sts_to_string(sts);
    }

    return Qnil;
}
