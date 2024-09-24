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
    char b = 'c';
    int8_t c = 'C';
    uint8_t d = 'D';
    int16_t e = 0xccbb;
    uint16_t f = 0x00bb;
    int32_t g = 0xFF;
    uint32_t h = 0xDDEE;
    int64_t i = 0xAABBCCDD;
    uint64_t j = 0x11223344;
    float fl = 3.14f;
    double db = 3.1415;

    std::string k = "Hello, World";
    std::vector<std::string> l = { "info", "warn", "error"};
    std::list<std::string> m = { "INFO", "WARN", "ERROR"};
    std::map<std::string, uint32_t> n = {
        {"H", 'H'},
        {"E", 'E'},
        {"L", 'L'},
        {"O", 'O'},
    };
    std::unordered_map<std::string, uint64_t> o = {
        {"H", 'H'},
        {"E", 'E'},
        {"L", 'L'},
        {"O", 'O'},
    };

    template<typename Archive>
    void save(Archive &ar) const
    {
        ar(a, b, c, d, e, f, g, h, i, j, fl, db, k, l, m, n, o);
    }

    template<typename Archive>
    void load(Archive &ar)
    {
        ar(a, b, c, d, e, f, g, h, i, j, fl, db, k, l, m, n, o);
    }

    void reset()
    {
        this->a = false;
        this->b = 0;
        this->c = 0;
        this->d = 0;
        this->e = 0;
        this->f = 0;
        this->g = 0;
        this->h = 0;
        this->i = 0;
        this->j = 0;
        this->fl = 0.0;
        this->db = 0.0;
        this->k.clear();
        this->l.clear();
        this->m.clear();
        this->n.clear();
        this->o.clear();
    }

    bool operator==(const Example &other)
    {
        return (this->a == other.a &&
            this->b == other.b &&
            this->c == other.c &&
            this->d == other.d &&
            this->e == other.e &&
            this->f == other.f &&
            this->g == other.g &&
            this->h == other.h &&
            this->i == other.i &&
            this->j == other.j &&
            this->fl == other.fl &&
            this->db == other.db &&
            this->k == other.k &&
            this->l == other.l &&
            this->m == other.m &&
            this->n == other.n &&
            this->o == other.o);
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
