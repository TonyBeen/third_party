/*************************************************************************
    > File Name: complex_example.cc
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月24日 星期二 10时35分17秒
 ************************************************************************/

#include <msgpackpp/msgpackpp.hpp>

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

struct ParentInfo
{
    std::string fatherName;
    uint16_t    fatherAge;
    std::string matherName;
    uint16_t    matherAge;

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(fatherName, fatherAge, matherName, matherAge);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(fatherName, fatherAge, matherName, matherAge);
    }

    bool operator==(const ParentInfo &other) const
    {
        return fatherName == other.fatherName && fatherAge == other.fatherAge &&
               matherName == other.matherName && matherAge == other.matherAge;
    }
};


struct ComplexType
{
    std::string other;
    ParentInfo parentInfo;
    std::map<std::string, std::string> nameMap;
    PersonInfo personInfo;

    // 不支持 std::map<std::string, PersonInfo> std::vector<PersonInfo> std::list<PersonInfo> std::unordered_map<T, PersonInfo>
    // 原因: msgpack库无法区分map中的键值对边界, 其存储方式为多对 [key - value], 而上述结构的值由多个成员组成, 破坏了msgpack的边界, 导致抛出异常
    // 所以, 复杂数据结构只支持基础类型, 目前这种序列化方式已满足自己开发需求, 不在扩展, 想要支持更复杂可选择 cereal
    // 可正常编译, 但是会抛出异常

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(other, parentInfo, nameMap, personInfo);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(other, parentInfo, nameMap, personInfo);
    }

    bool operator==(const ComplexType &obj)
    {
        return (this->other == obj.other && this->parentInfo == obj.parentInfo &&
                this->nameMap == obj.nameMap && this->personInfo == obj.personInfo);
    }
};

void print(const void *buf,size_t len)
{
    const char *buffer = (const char *)buf;
    size_t i = 0;
    for(; i < len ; ++i) {
        printf("%02x ", 0xff & buffer[i]);
        if (i && (i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    ComplexType cmp;
    cmp.other = "XXX";
    cmp.parentInfo.fatherName = "VV";
    cmp.parentInfo.fatherAge = 34;
    cmp.parentInfo.matherName = "BB";
    cmp.parentInfo.matherAge = 32;
    cmp.nameMap = {
        {"AAA", "AAA"},
        {"BBB", "BBB"},
        {"CCC", "CCC"}
    };

    cmp.personInfo = {
        "YYY", 22
    };

    msgpack::MsgPackBinary pack;
    pack(cmp);

    print(pack.buffer(), pack.size());

    ComplexType other;
    msgpack::MsgUnpackBinary unpack(pack.buffer(), pack.size());
    unpack(other);

    if (cmp == other) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    // 如果想复用 unpack, 需要调用reset后在次解析
    unpack.reset(pack.buffer(), pack.size());
    ComplexType other_2;
    unpack(other_2);

    if (cmp == other_2) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    return 0;
}
