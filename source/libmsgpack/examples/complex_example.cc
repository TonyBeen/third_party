/*************************************************************************
    > File Name: complex_example.cc
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月24日 星期二 10时35分17秒
 ************************************************************************/

#include <msgpackpp.hpp>

struct PersonInfo
{
    std::string address;
    uint32_t    age;

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(address, age);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(address, age);
    }

    bool operator==(const PersonInfo &other) const
    {
        return address == other.address && age == other.age;
    }
};

struct ComplexType
{
    std::map<std::string, PersonInfo> nameMap;

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(nameMap);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(nameMap);
    }
};

int main(int argc, char **argv)
{
    ComplexType cmp;
    cmp.nameMap = {
        {"Alice", {"123 Maple St, CityA", 30}},
        {"Bob", {"456 Oak St, CityB", 25}},
        {"Charlie", {"789 Pine St, CityC", 35}}
    };

    msgpack::MsgPackBinary pack;
    pack(cmp);

    ComplexType other;
    msgpack::MsgUnpackBinary unpack(pack.buffer(), pack.size());
    unpack(other);

    if (cmp.nameMap == other.nameMap) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    return 0;
}
