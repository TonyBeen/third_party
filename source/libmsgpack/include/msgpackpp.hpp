/*************************************************************************
    > File Name: msgpack.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月22日 星期日 17时30分09秒
 ************************************************************************/

#ifndef __MSGPACK_H__
#define __MSGPACK_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>

#include <archive.hpp>

namespace msgpack {
class MsgPackBinary : public OutputArchive<MsgPackBinary>
{
    MsgPackBinary(const MsgPackBinary&) = default;
    MsgPackBinary&operator=(const MsgPackBinary&) = default;

public:
    MsgPackBinary() : OutputArchive(this) { }
    ~MsgPackBinary() = default;

    void *buffer() const
    {
        return _sbuf.data;
    }

    size_t size() const
    {
        return _sbuf.size;
    }
};

class MsgUnpackBinary : public InputArchive<MsgUnpackBinary>
{
    MsgUnpackBinary(const MsgUnpackBinary&) = default;
    MsgUnpackBinary&operator=(const MsgUnpackBinary&) = default;

public:
    MsgUnpackBinary(const void *data, size_t size) :
        InputArchive(this)
    {
        _buffer = data;
        _size = size;
    }

    ~MsgUnpackBinary() = default;
};

} // namespace eular

#endif // __MSGPACK_H__