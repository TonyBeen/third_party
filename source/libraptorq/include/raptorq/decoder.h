/*************************************************************************
    > File Name: decoder.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月10日 星期二 10时13分22秒
 ************************************************************************/

#ifndef __RAPTOR_Q_DECODER_H__
#define __RAPTOR_Q_DECODER_H__

#include <vector>
#include <memory>

namespace raptorq {
struct DecoderPrivate;
class Decoder
{
    // disallow copy and move constructor
    Decoder(const Decoder&) = default;
    Decoder &operator=(const Decoder&) = default;
    Decoder(Decoder&&) = delete;
    Decoder &operator=(Decoder&&) = delete;

public:
    using SP = std::shared_ptr<Decoder>;
    using WP = std::weak_ptr<Decoder>;
    using Ptr = std::unique_ptr<Decoder>;

    Decoder();
    Decoder(uint32_t piece_per_block, uint16_t piece_size);
    ~Decoder();

    bool reset(uint32_t piece_per_block = 0, uint16_t piece_size = 0);

    /**
     * @brief 添加解码数据
     *
     * @param data 数据缓存
     * @param id id
     * @return true 成功返回true
     * @return false 当添加的解码数据都为源编码数据时再次尝试添加修复符号数据时会报错, 此时可尝试解码
     */
    bool feedPiece(const void *data, uint32_t id);

    /**
     * @brief 添加解码数据
     *
     * @param data 数据缓存
     * @param id id
     */
    bool feedPiece(const std::string &data, uint32_t id);

    /**
     * @brief 预处理数据
     *
     * @param threads 线程个数
     */
    void precompute(uint8_t threads);

    /**
     * @brief 获取解码后的数据
     *
     * @param source_data 源数据缓存
     * @return true 
     * @return false 
     */
    bool decode(std::string &source_data);

private:
    std::unique_ptr<DecoderPrivate> m_decoder;
};

} // namespace raptorq

#endif // __RAPTOR_Q_DECODER_H__
