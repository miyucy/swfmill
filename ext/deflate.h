#ifndef _DEFLATE_H_
#define _DEFLATE_H_
#include <zlib.h>
#include "buffer.h"

class Deflate
{
public:

    Deflate(const unsigned char* data, const size_t size, int compress_level=Z_BEST_COMPRESSION)
        : _initialized(false), _temp(NULL), _compress_level(compress_level)
    {
        _data = const_cast<unsigned char*>(data);
        _data_size = size;
        _temp = new Bytef[size];
        _temp_size = size;
    }

    ~Deflate()
    {
        finish();
        delete[] _temp;
    }

    bool compress()
    {
        if(!init())
        {
            return false;
        }

        for(;;)
        {
            if(stream.avail_in == 0)
            {
                break;
            }

            if(deflate(&stream, Z_NO_FLUSH) != Z_OK)
            {
                return false;
            }

            size_t count = _temp_size - stream.avail_out;
            if(count > 0)
            {
                _buff.append(_temp, count);
            }

            stream.next_out  = _temp;
            stream.avail_out = _temp_size;
        }

        int status;
        do
        {
            status = deflate(&stream, Z_FINISH);

            size_t count = _temp_size - stream.avail_out;
            if(count > 0)
            {
                _buff.append(_temp, count);
            }

            stream.next_out  = _temp;
            stream.avail_out = _temp_size;
        } while(status == Z_OK);

        return status == Z_STREAM_END;
    }

    const char* error() const
    {
        return stream.msg;
    }

    const Bytef* data() const
    {
        return _buff.ptr();
    }

    const size_t size() const
    {
        return _buff.size();
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
        stream.next_out  = _temp;
        stream.avail_out = _temp_size;
        return _initialized = deflateInit(&stream, _compress_level) == Z_OK;
    }

    bool finish()
    {
        if(_initialized)
        {
            return deflateEnd(&stream) == Z_OK;
        }
        return false;
    }

    z_stream      stream;
    buffer<Bytef>  _buff;
    unsigned char* _data;
    size_t    _data_size;
    Bytef*         _temp;
    size_t    _temp_size;
    bool    _initialized;
    int  _compress_level;

};

#endif /* _DEFLATE_H_ */
