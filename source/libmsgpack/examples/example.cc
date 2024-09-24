/*************************************************************************
    > File Name: example.cc
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月23日 星期一 20时02分58秒
 ************************************************************************/

#include <msgpackpp.hpp>

struct Example
{
    bool a = true;
    int32_t b = 0xFF;
    uint32_t c = 0xDDEE;
    int64_t d = 0xAABBCCDD;
    uint64_t e = 0x11223344;

    std::string f = "Hello, World";
    std::vector<std::string> g = { "info", "warn", "error"};
    std::list<std::string> h = { "INFO", "WARN", "ERROR"};
    std::map<std::string, uint32_t> l = {
        {"H", 'H'},
        {"E", 'E'},
        {"L", 'L'},
        {"O", 'O'},
    };
    std::unordered_map<std::string, uint64_t> m = {
        {"H", 'H'},
        {"E", 'E'},
        {"L", 'L'},
        {"O", 'O'},
    };

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(a, b, c, d, e, f, g, h, l, m);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(a, b, c, d, e, f, g, h, l, m);
    }

    void reset()
    {
        this->a = false;
        this->b = 0;
        this->c = 0;
        this->d = 0;
        this->e = 0;
        this->f = "";
        this->g.clear();
        this->h.clear();
        this->l.clear();
        this->m.clear();
    }

    bool operator==(const Example &other)
    {
        return a == other.a &&
            b == other.b &&
            c == other.c &&
            d == other.d &&
            e == other.e &&
            f == other.f &&
            g == other.g &&
            h == other.h &&
            l == other.l &&
            m == other.m;
    }
};

int main(int argc, char **argv)
{
    msgpack::MsgPackBinary pack;
    Example ex;
    pack(ex);

    msgpack::MsgUnpackBinary unpack(pack.buffer(), pack.size());
    Example other;
    other.reset();
    unpack(other);

    if (ex == other) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    return 0;
}
