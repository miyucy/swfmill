#include <cstdio>
#include <cstring>
#include <algorithm>
#include <memory>
#include <zlib.h>
#include "ruby.h"

template <class T>
class buffer
{
public:

    buffer(const size_t init)
        : offset_(0), size_(init)
    {
        data_ = new T[size_];
    }

    ~buffer()
    {
        delete[] data_;
    }

    void append(const T* const src, const size_t len)
    {
        expand(len);
        std::memcpy(data_ + offset_, src, len);
        offset_ += len;
    }

    T* ptr()
    {
        return data_;
    }

    const size_t size()
    {
        return offset_;
    }

    const size_t capacity()
    {
        return size_;
    }

private:

    void expand(const size_t len)
    {
        if(!need_expand(len))
        {
            return;
        }

        size_t new_size = size_ + std::max<size_t>(len * 2, size_ / 2);
        T*    new_place = new T[new_size];

        std::memcpy(new_place, data_, size_);
        delete[] data_;

        data_ = new_place;
        size_ = new_size;
    }

    bool need_expand(const size_t len)
    {
        return offset_ + len >= size_;
    }

    T*     data_;
    size_t offset_, size_;
};

int swfmill_ext_deflate(unsigned char*& buff, const size_t size, const int compress_level)
{
    buffer<Bytef>tp(1024);

    z_stream st  = {Z_NULL};
    st.next_in   = buff;
    st.avail_in  = size;
    st.next_out  = tp.ptr();
    st.avail_out = tp.capacity();

    if(deflateInit(&st, compress_level) != Z_OK)
    {
        return 0;
    }

    buffer<Bytef>ob(size);

    for(;;)
    {
        if(st.avail_in == 0)
        {
            break;
        }

        if(deflate(&st, Z_NO_FLUSH) != Z_OK)
        {
            return 0;
        }

        size_t count = tp.capacity() - st.avail_out;
        if(count > 0)
        {
            ob.append(tp.ptr(), count);
        }

        st.next_out  = tp.ptr();
        st.avail_out = tp.capacity();
    }

    int status;
    do
    {
        status = deflate(&st, Z_FINISH);

        size_t count = tp.capacity() - st.avail_out;
        if(count > 0)
        {
            ob.append(tp.ptr(), count);
        }

        st.next_out  = tp.ptr();
        st.avail_out = tp.capacity();
    } while(status == Z_OK);

    if(status != Z_STREAM_END)
    {
        return 0;
    }

    if(deflateEnd(&st) != Z_OK)
    {
        return 0;
    }

    xfree(buff);
    buff = ALLOC_N(Bytef, ob.size());
    std::memcpy(buff, ob.ptr(), ob.size());
    return ob.size();
}
