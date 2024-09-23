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

#include <msgpack.h>

namespace msgpack {
class MsgPack final
{
public:
    MsgPack() {}
    ~MsgPack() {}

    MsgPack &operator<<(const std::string &msg);

    template<typename Key, typename Val>
    MsgPack &operator<<(const std::pair<Key, Val> &msgPair);

    template<typename Key, typename Val>
    MsgPack &operator<<(const std::map<Key, Val> &msgMap);

    template<typename Key, typename Val>
    MsgPack &operator<<(const std::unordered_map<Key, Val> &msgHashMap);

    template<typename Val>
    MsgPack &operator<<(const std::vector<Val> &msgVec);

    template<typename Val>
    MsgPack &operator<<(const std::list<Val> &msgList);

private:

};

MsgPack &msgpack::MsgPack::operator<<(const std::string &msg)
{
    return *this;
}

template <typename Key, typename Val>
inline MsgPack &MsgPack::operator<<(const std::pair<Key, Val> &msgPair)
{
    return *this;
}

template <typename Key, typename Val>
inline MsgPack &MsgPack::operator<<(const std::map<Key, Val> &msgMap)
{
    return *this;
}

template <typename Key, typename Val>
inline MsgPack &MsgPack::operator<<(const std::unordered_map<Key, Val> &msgHashMap)
{
    return *this;
}

template <typename Val>
inline MsgPack &MsgPack::operator<<(const std::vector<Val> &msgVec)
{
    return *this;
}

template <typename Val>
inline MsgPack &MsgPack::operator<<(const std::list<Val> &msgList)
{
    return *this;
}

} // namespace eular

#endif // __MSGPACK_H__