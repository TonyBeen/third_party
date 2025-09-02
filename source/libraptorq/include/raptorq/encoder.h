/*************************************************************************
    > File Name: encoder.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月10日 星期二 10时13分10秒
 ************************************************************************/

#ifndef __RAPTOR_Q_ENCODER_H__
#define __RAPTOR_Q_ENCODER_H__

#include <vector>
#include <memory>

namespace raptorq {
struct EncoderPrivate;
class Encoder
{
    // disallow copy and move constructor
    Encoder(const Encoder&) = default;
    Encoder &operator=(const Encoder&) = default;
    Encoder(Encoder&&) = delete;
    Encoder &operator=(Encoder&&) = delete;

public:
    using SP = std::shared_ptr<Encoder>;
    using WP = std::weak_ptr<Encoder>;
    using Ptr = std::unique_ptr<Encoder>;

    Encoder();
    Encoder(uint32_t piece_per_block, uint16_t piece_size, uint8_t redundancy_piece, const void *block_data);
    ~Encoder();

    /**
     * @brief 重置编码数据
     * 
     * @param piece_per_block 
     * @param piece_size 
     * @param redundancy_piece 
     * @param block_data 
     * @return true 成功
     * @return false 失败
     */
    bool reset(uint32_t piece_per_block = 0, uint16_t piece_size = 0,
               uint8_t redundancy_piece = 0, const void *block_data = nullptr);

    /**
     * @brief 预处理数据
     * 
     * @param threads 线程个数
     */
    void precompute(uint8_t threads = 1);

    /**
     * @brief 获取编码后的数据
     * 
     * @param id_vec id数组
     * @param data_vec 编码后数据数组
     * @return true 
     * @return false 
     */
    bool encode(std::vector<uint32_t> &id_vec, std::vector<void *> &data_vec);

private:
    std::unique_ptr<EncoderPrivate> m_encoder;
};

} // namespace raptorq

#endif // __RAPTOR_Q_ENCODER_H__
