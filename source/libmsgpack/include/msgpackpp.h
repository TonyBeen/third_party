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
#include <initializer_list>
#include <map>
#include <unordered_map>

#include <msgpack.h>

namespace msgpack {
class MsgPack
{
public:
    MsgPack();
    ~MsgPack() {}

    MsgPack &operator<<(const std::string &);

private:

};

} // namespace eular

#endif // __MSGPACK_H__