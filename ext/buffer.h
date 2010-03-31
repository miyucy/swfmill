#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <cstring>
#include <algorithm>

template <class T>
class buffer
{
public:

    buffer(size_t init=INITIAL_SIZE)
        : _data(NULL), _size(0), _capacity(init)
    {
        _data = new T[init];
    }

    buffer(const buffer<T>& other)
        : _data(NULL), _size(0), _capacity(0)
    {
        _data = new T[other.size()];
        _capacity = other.size();
        append(other.ptr(), other.size());
    }

    virtual ~buffer()
    {
        delete[] _data;
    }

    const T* ptr() const
    {
        return _data;
    }

    const size_t size() const
    {
        return _size;
    }

    const size_t capacity() const
    {
        return _capacity;
    }

    const size_t reserve(const size_t length)
    {
        T* temp;

        _capacity = length;
        _size = std::min<size_t>(_size, length);

        temp = new T[_capacity];
        std::memcpy(temp, _data, _size);
        delete[] _data;

        _data = temp;
        return _capacity;
    }

    const size_t resize(const size_t length)
    {
        expand(length - _size);
        return _size = length;
    }

    size_t append(const T* const source, const size_t length)
    {
        expand(length);
        std::memcpy(_data + _size, source, length);
        return _size += length;
    }

private:

    void expand(size_t length)
    {
        if(need_reserve(length))
        {
            reserve(_capacity + length + length / 2);
        }
    }

    bool need_reserve(size_t length) const
    {
        return _size + length >= _capacity;
    }

    static const size_t INITIAL_SIZE = 255;
    T*         _data;
    size_t     _size;
    size_t _capacity;

};

#endif /* _BUFFER_H_ */
