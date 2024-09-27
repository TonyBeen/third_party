/*************************************************************************
    > File Name: arcgive.hpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月23日 星期一 11时49分18秒
 ************************************************************************/

#ifndef __MSGPACK_ARCGIVE_H__
#define __MSGPACK_ARCGIVE_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <exception>

#include <msgpack.h>
#include <traits.hpp>

#define DATA_TYPE_MISMATCH(Type) \
    throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + ", with [T = " + traits::ClassNameHelper<Type>::Name + "], data type mismatch");

namespace msgpack {
HAS_MEMBER_CONST(detail, save);
template<typename ArchiveType>
class OutputArchive
{
    OutputArchive(const OutputArchive &) = delete;
    OutputArchive& operator=(const OutputArchive &) = delete;
public:
    static const bool IS_OUTPUT_ARCHIVE = true;

    OutputArchive(ArchiveType *derived) : _self(derived)
    {
        msgpack_sbuffer_init(&_sbuf);
        msgpack_packer_init(&_pk, &_sbuf, msgpack_sbuffer_write);
    }

    virtual ~OutputArchive()
    {
        reset();
    }

    template <typename ... Types> inline
    void operator()(Types && ... args)
    {
        if (sizeof...(args) == 0) {
            return;
        }

        _self->process(std::forward<Types>(args)...);
    }

    void reset()
    {
        msgpack_sbuffer_destroy(&_sbuf);
        msgpack_sbuffer_init(&_sbuf);
        msgpack_packer_init(&_pk, &_sbuf, msgpack_sbuffer_write);
    }

private:
    template <typename T> inline
    void process(T && head)
    {
        _self->processImpl(head);
    }

    // Unwinds to process all data
    template <typename T, typename ... Other> inline
    void process(T && head, Other && ... tail)
    {
        _self->process(std::forward<T>(head));
        _self->process(std::forward<Other>(tail)...);
    }

private:
    template<typename T>
    void processImpl(const T &other)
    {
        static_assert(detail::has_member_save<T, void, ArchiveType &>::value,
            "The 'template<typename ArchiveType> void T::save(ArchiveType &)' function must be implemented");
        other.save(*_self);
    }

    // 基础类型
    void processImpl(const bool &msg)
    {
        if (msg) {
            msgpack_pack_true(&_pk);
        } else {
            msgpack_pack_false(&_pk);
        }
    }

    // char 和 signed char是两种类型
    void processImpl(const char &msg)
    {
        msgpack_pack_int64(&_pk, msg);
    }

    void processImpl(const int8_t &msg)
    {
        msgpack_pack_int64(&_pk, msg);
    }

    void processImpl(const uint8_t &msg)
    {
        msgpack_pack_uint64(&_pk, msg);
    }

    void processImpl(const int16_t &msg)
    {
        msgpack_pack_int64(&_pk, msg);
    }

    void processImpl(const uint16_t &msg)
    {
        msgpack_pack_uint64(&_pk, msg);
    }

    void processImpl(const int32_t &msg)
    {
        msgpack_pack_int64(&_pk, msg);
    }

    void processImpl(const uint32_t &msg)
    {
        msgpack_pack_uint64(&_pk, msg);
    }

    void processImpl(const int64_t &msg)
    {
        msgpack_pack_int64(&_pk, msg);
    }

    void processImpl(const uint64_t &msg)
    {
        msgpack_pack_uint64(&_pk, msg);
    }

    void processImpl(const float &msg)
    {
        msgpack_pack_double(&_pk, msg);
    }

    void processImpl(const double &msg)
    {
        msgpack_pack_double(&_pk, msg);
    }

    // 复杂类型
    template<typename CharT>
    void processImpl(const std::basic_string<CharT> &msg)
    {
        size_t size = msg.size() * sizeof(CharT);
        msgpack_pack_str(&_pk, size);
        msgpack_pack_str_body(&_pk, msg.c_str(), size);
    }

    template<typename Key, typename Val>
    void processImpl(const std::pair<Key, Val> &msg)
    {
        msgpack_pack_map(&_pk, 1);
        processImpl(msg.first);
        processImpl(msg.second);
    }

    template<typename Key, typename Val>
    void processImpl(const std::pair<Key, Val> &msg, std::true_type)
    {
        processImpl(msg.first);
        processImpl(msg.second);
    }

    template<typename Val, size_t N>
    void processImpl(const std::array<Val, N> &msg)
    {
        msgpack_pack_array(&_pk, N);
        for (size_t i = 0; i < N; ++i) {
            processImpl(msg[i]);
        }
    }

    template<typename Val>
    void processImpl(const std::vector<Val> &msg)
    {
        size_t size = msg.size();
        msgpack_pack_array(&_pk, msg.size());
        for (const auto &it : msg) {
            processImpl(it);
        }
    }

    template<typename Val>
    void processImpl(const std::list<Val> &msg)
    {
        size_t size = msg.size();
        msgpack_pack_array(&_pk, msg.size());
        for (const auto &it : msg) {
            processImpl(it);
        }
    }

    template<typename Key, typename Val, typename Compare = std::less<Key>>
    void processImpl(const std::map<Key, Val, Compare> &msg)
    {
        size_t size = msg.size();
        msgpack_pack_map(&_pk, size);
        for (const auto &it : msg) {
            processImpl<Key, Val>(it, std::true_type());
        }
    }

    template<typename Key, typename Val, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
    void processImpl(const std::unordered_map<Key, Val, Hash, Pred> &msg)
    {
        size_t size = msg.size();
        msgpack_pack_map(&_pk, size);
        for (const auto &it : msg) {
            processImpl<Key, Val>(it, std::true_type());
        }
    }

protected:
    ArchiveType*    _self;
    msgpack_sbuffer _sbuf;
    msgpack_packer  _pk;
};

HAS_MEMBER(detail, load);
template<typename ArchiveType>
class InputArchive
{
    InputArchive& operator=(const InputArchive &) = delete;
public:
    static const bool IS_OUTPUT_ARCHIVE = false;

    InputArchive(ArchiveType *derived) : _self(derived)
    {
        msgpack_unpacked_init(&_unpack);
    }

    virtual ~InputArchive()
    {
        reset();
    }

    template <typename ... Types> inline
    void operator()(Types && ... args)
    {
        if (sizeof...(args) == 0) {
            return;
        }

        if (_buffer == nullptr || _size == 0) {
            return;
        }

        _self->process(std::forward<Types>(args)...);
    }

    void reset(const void *data = nullptr, size_t size = 0)
    {
        _offset = 0;
        _buffer = (const char *)data;
        _size = size;

        msgpack_unpacked_destroy(&_unpack);
        msgpack_unpacked_init(&_unpack);        
    }

private:
    template <typename T> inline
    void process(T && head)
    {
        _self->processImpl(head);
    }

    // Unwinds to process all data
    template <typename T, typename ... Other> inline
    void process(T && head, Other && ... tail)
    {
        _self->process(std::forward<T>(head));
        _self->process(std::forward<Other>(tail)...);
    }

private:
    template<typename T>
    void processImpl(T &obj, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr)
    {
        static_assert(detail::has_member_load<T, void, ArchiveType &>::value,
            "The 'template<typename ArchiveType> void T::load(ArchiveType &)' function must be implemented");
        obj.load(*_self);
    }

    // 基础类型
    template<typename T>
    void processImpl(T &msg, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        internalProcess(_unpack.data, msg);
    }

    // 复杂类型
    template<typename CharT>
    void processImpl(std::basic_string<CharT> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_STR) {
            const msgpack_object_str &str = obj.via.str;
            msg.append(str.ptr, str.size);
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Key, typename Val>
    void processImpl(std::pair<Key, Val> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_MAP && obj.via.map.size == 1) {
            const msgpack_object_map &objMap = obj.via.map;
            internalProcess(objMap, 0, msg);
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Val, size_t N>
    void processImpl(std::array<Val, N> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size == N) {
            const msgpack_object_array &objArray = obj.via.array;
            for (size_t i = 0; i < N; ++i) {
                internalProcess(objArray.ptr[i], msg[i]);
            }
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Val>
    void processImpl(std::vector<Val> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY) {
            msg.resize(obj.via.array.size);
            const msgpack_object_array &objArray = obj.via.array;
            for (size_t i = 0; i < obj.via.array.size; ++i) {
                internalProcess(objArray.ptr[i], msg[i]);
            }
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Val>
    void processImpl(std::list<Val> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY) {
            const msgpack_object_array &objArray = obj.via.array;
            for (size_t i = 0; i < obj.via.array.size; ++i) {
                Val temp;
                internalProcess(objArray.ptr[i], temp);
                msg.push_back(std::move(temp));
            }
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Key, typename Val, typename Compare = std::less<Key>>
    void processImpl(std::map<Key, Val, Compare> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_MAP) {
            const msgpack_object_map &objMap = obj.via.map;
            for (auto i = 0; i < obj.via.map.size; ++i) {
                std::pair<Key, Val> temp;
                internalProcess(objMap, i, temp);
                msg.insert(std::move(temp));
            }
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Key, typename Val, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
    void processImpl(std::unordered_map<Key, Val, Hash, Pred> &msg)
    {
        msgpack_unpack_return code = msgpack_unpack_next(&_unpack, _buffer, _size, &_offset);
        if (code != MSGPACK_UNPACK_SUCCESS) {
            throw std::runtime_error("parse failed");
        }

        const msgpack_object &obj = _unpack.data;
        if (obj.type == MSGPACK_OBJECT_MAP) {
            const msgpack_object_map &objMap = obj.via.map;
            for (auto i = 0; i < obj.via.map.size; ++i) {
                std::pair<Key, Val> temp;
                internalProcess(objMap, i, temp);
                msg.insert(std::move(temp));
            }
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

private:
    template<typename T>
    void internalProcess(const msgpack_object &obj, T &msg, typename std::enable_if<!std::is_arithmetic<T>::value>::type * = nullptr)
    {
        // TODO 如果是T复杂类型, 加载时需要从obj加载
        // 存储时是顺序存储, 会将一个结构体成员拆分成多个, 导致存储结构体map时会出现偏差
        static_assert(detail::has_member_load<T, void, ArchiveType &>::value,
            "The 'template<typename ArchiveType> void T::load(ArchiveType &)' function must be implemented");
        msg.load(*_self);
    }

    // 处理基础类型值
    template<typename T>
    void internalProcess(const msgpack_object &obj, T &msg, typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr)
    {
        switch (obj.type) {
        case MSGPACK_OBJECT_BOOLEAN:
            msg = obj.via.boolean;
            break;
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            msg = static_cast<T>(obj.via.u64);
            break;
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            msg = static_cast<T>(obj.via.i64);
            break;
        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64:
            msg = static_cast<T>(obj.via.f64);
            break;
        default:
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
            break;
        }
    }

    template<typename CharT>
    void internalProcess(const msgpack_object &obj, std::basic_string<CharT> &msg)
    {
        if (obj.type == MSGPACK_OBJECT_STR) {
            assert(obj.via.str.size % sizeof(CharT) == 0);
            msg.append(reinterpret_cast<const CharT *>(obj.via.str.ptr), obj.via.str.size / sizeof(CharT)); 
        } else {
            DATA_TYPE_MISMATCH(traits::RemoveConstRef<decltype(msg)>);
        }
    }

    template<typename Key, typename Val>
    void internalProcess(const msgpack_object_map &objMap, size_t index, std::pair<Key, Val> &objPair)
    {
        assert(objMap.size > index);
        const msgpack_object_kv& obj = objMap.ptr[index];

        internalProcess(obj.key, objPair.first);
        internalProcess(obj.val, objPair.second);
    }

protected:
    ArchiveType*        _self{nullptr};
    const char *        _buffer{nullptr};
    size_t              _size{0};
    size_t              _offset{0};
    msgpack_unpacked    _unpack;
};

} // namespace msgpack

#endif // __MSGPACK_ARCGIVE_H__
