#include "ruby.h"
#include "SWF.h"
#include "SWFReader.h"
#include "inflate.h"

extern VALUE rb_eSwfmill_Error;
extern VALUE rb_eSwfmill_EOFError;

#define SIGNATURE_SIZE 8
typedef struct {
    VALUE          string;
    VALUE        encoding;
    bool       compressed;
    SWF::Header   *header;
    SWF::Context *context;
    size_t       datasize;
} swfmill_to_xml;

static void
stx_mark(swfmill_to_xml * const stx)
{
    if(stx != NULL)
    {
        rb_gc_mark(stx->string);
        rb_gc_mark(stx->encoding);
    }
}

static void
stx_free(swfmill_to_xml * const stx)
{
    if(stx != NULL)
    {
        delete stx->header;
        stx->header = NULL;
        delete stx->context;
        stx->context = NULL;
    }
}

static bool
stx_validate(const swfmill_to_xml * const stx)
{
    if((RSTRING_LEN(stx->string) < SIGNATURE_SIZE) ||
       (strncmp(RSTRING_PTR(stx->string), "CWS", 2) != 0 &&
        strncmp(RSTRING_PTR(stx->string), "FWS", 2) != 0))
    {
        rb_raise(rb_eSwfmill_Error, "input is no SWF");
        return false;
    }
    return true;
}

static void
stx_check_version(swfmill_to_xml * const stx)
{
    stx->context = new SWF::Context;
    stx->context->debugTrace = false;
    stx->context->quiet      = true;

    stx->context->swfVersion = (int)RSTRING_PTR(stx->string)[3];
    // fprintf(stderr, "swfVersion: %d\n", stx->context->swfVersion);
}

static void
stx_check_compressed(swfmill_to_xml * const stx)
{
    stx->compressed = RSTRING_PTR(stx->string)[0] == 'C';
    // fprintf(stderr, "compressed: %d\n", stx->compressed);
}

static void
stx_check_datasize(swfmill_to_xml * const stx)
{
    stx->datasize  = (size_t)RSTRING_PTR(stx->string)[4];
    stx->datasize += (size_t)RSTRING_PTR(stx->string)[5] << 8;
    stx->datasize += (size_t)RSTRING_PTR(stx->string)[6] << 16;
    stx->datasize += (size_t)RSTRING_PTR(stx->string)[7] << 24;
    // fprintf(stderr, "datasize: %d\n", stx->datasize);

    if(stx->datasize != RSTRING_LEN(stx->string) - SIGNATURE_SIZE)
    {
        if((!stx->compressed) &&
           (stx->datasize > RSTRING_LEN(stx->string) - SIGNATURE_SIZE))
        {
            stx->datasize = RSTRING_LEN(stx->string) - SIGNATURE_SIZE;
        }
    }
    // fprintf(stderr, "fixed datasize: %d\n", stx->datasize);
}

static bool
stx_copy(swfmill_to_xml * const stx)
{
    stx->string = rb_str_new(RSTRING_PTR(stx->string) + SIGNATURE_SIZE,
                             RSTRING_LEN(stx->string) - SIGNATURE_SIZE);
    return true;
}

static bool
stx_decompress(swfmill_to_xml * const stx)
{
    if(!stx->compressed)
    {
        return stx_copy(stx);
    }

    const unsigned char* buff = (const unsigned char*)RSTRING_PTR(stx->string) + SIGNATURE_SIZE;
    const size_t         size = RSTRING_LEN(stx->string) -  SIGNATURE_SIZE;
    Inflate inflate(buff, size, stx->datasize);

    if(!inflate.decompress())
    {
        rb_raise(rb_eSwfmill_Error, "decompressing SWF");
        return false;
    }

    stx->string = rb_str_new((const char*)inflate.data(), stx->datasize);

    return true;
}

static bool
stx_read(swfmill_to_xml* const stx)
{
    SWF::Reader reader((const unsigned char*)RSTRING_PTR(stx->string), RSTRING_LEN(stx->string));

    stx->header = new SWF::Header;
    stx->header->parse(&reader, RSTRING_LEN(stx->string), stx->context);

    switch(reader.getError())
    {
    case SWFR_ERROR:
        rb_raise(rb_eSwfmill_Error, "unknown error while reading SWF");
        return false;
    case SWFR_EOF:
        rb_raise(rb_eSwfmill_EOFError, "reached EOF while reading SWF");
        return false;
    }

    return true;
}

static bool
stx_parse(swfmill_to_xml* const stx)
{
    if(!stx_validate(stx))
    {
        return false;
    }
    stx_check_version(stx);
    stx_check_compressed(stx);
    stx_check_datasize(stx);
    if(!stx_decompress(stx))
    {
        return false;
    }
    if(!stx_read(stx))
    {
        return false;
    }
    return true;
}

static void
stx_init_xml_memory()
{
    xmlMemSetup((xmlFreeFunc)ruby_xfree,
                (xmlMallocFunc)ruby_xmalloc,
                (xmlReallocFunc)ruby_xrealloc,
                strdup);
}

static void
stx_set_version_proparty(const swfmill_to_xml* const stx, xmlNodePtr root)
{
    char temp[4];
    snprintf(temp, sizeof(temp), "%i", stx->context->swfVersion);
    xmlSetProp(root, (const xmlChar*)"version", (const xmlChar*)temp);
}

static void
stx_set_compressed_proparty(const swfmill_to_xml* const stx, xmlNodePtr root)
{
    char temp[4];
    snprintf(temp, sizeof(temp), "%i", stx->compressed ? 1 : 0);
    xmlSetProp(root, (const xmlChar*)"compressed", (const xmlChar*)temp);
}

static void
stx_set_encoding(swfmill_to_xml* const stx)
{
    stx->context->convertEncoding = true;
    stx->context->swf_encoding    = StringValueCStr(stx->encoding);
    // fprintf(stderr, "%s\n", stx->context->swf_encoding);
}

static VALUE
stx_dump(const swfmill_to_xml* const stx, xmlDocPtr doc)
{
    VALUE      xml = Qnil;
    char* xml_data = NULL;
    int   xml_size = 0;

    xmlDocDumpMemoryEnc(doc, (xmlChar**)&xml_data, &xml_size, "UTF-8");
    if(xml_size > 0)
    {
        xml = rb_str_new(xml_data, xml_size);
    }
    if(xml_data != NULL)
    {
        xmlFree(xml_data);
    }
    return xml;
}

static VALUE
stx_to_string(swfmill_to_xml* const stx)
{
    VALUE       xml = Qnil;
    xmlDocPtr   doc;
    xmlNodePtr root;
    stx_init_xml_memory();

    doc  = xmlNewDoc((const xmlChar*)"1.0");
    root = doc->xmlRootNode = xmlNewDocNode(doc, NULL, (const xmlChar*)"swf", NULL);

    stx_set_version_proparty(stx, root);
    stx_set_compressed_proparty(stx, root);
    stx_set_encoding(stx);

    stx->header->writeXML(root, stx->context);
    xml = stx_dump(stx, doc);

    xmlFreeDoc(doc);
    return xml;
}

/*
  call-seq:
    Swfmill.to_xml(string, encoding) -> String
 */
VALUE
swfmill_ext_to_xml(VALUE self, VALUE string, VALUE encoding)
{
    swfmill_to_xml *stx;

    Check_Type(string, T_STRING);
    Check_Type(encoding, T_STRING);

    Data_Make_Struct(rb_cData, swfmill_to_xml, stx_mark, stx_free, stx);

    stx->string   = string;
    stx->encoding = encoding;

    if(stx_parse(stx))
    {
        return stx_to_string(stx);
    }

    return Qnil;
}
