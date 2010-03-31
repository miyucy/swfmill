#ifndef _INFLATE_H_
#define _INFLATE_H_
#include <zlib.h>

class Inflate
{
public:

    Inflate(const unsigned char* data, const size_t size, const size_t length)
        : _initialized(false)
    {
        _data = const_cast<unsigned char*>(data);
        _data_size = size;
        _buff = new Bytef[length];
        _buff_size = length;
    }

    ~Inflate()
    {
        finish();
        delete[] _buff;
    }

    bool decompress()
    {
        if(!init())
        {
            return false;
        }

        int status;
        status = inflate(&stream, Z_FINISH);
        return status == Z_STREAM_END;
    }

    const char* error() const
    {
        return stream.msg;
    }

    const Bytef* data() const
    {
        return _buff;
    }

private:

    bool init()
    {
        if(_initialized)
        {
            return true;
        }
        stream.zalloc    = Z_NULL;
        stream.zfree     = Z_NULL;
        stream.opaque    = Z_NULL;
        stream.next_in   = _data;
        stream.avail_in  = _data_size;
        stream.next_out  = _buff;
        stream.avail_out = _buff_size;
        return _initialized = inflateInit(&stream) == Z_OK;
    }

    bool finish()
    {
        if(_initialized)
        {
            return inflateEnd(&stream) == Z_OK;
        }
        return false;
    }

    z_stream      stream;
    unsigned char* _data;
    Bytef*         _buff;
    size_t    _data_size;
    size_t    _buff_size;
    bool    _initialized;

};

#endif /* _INFLATE_H_ */
