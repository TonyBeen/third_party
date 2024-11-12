/*************************************************************************
    > File Name: raptorq.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月10日 星期二 10时15分36秒
 ************************************************************************/

#ifndef __RAPTOR_Q_H__
#define __RAPTOR_Q_H__

#include <stdint.h>
#include <stdbool.h>

#define MIN_PIECE_SIZE  64

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief 此编码器不管多少字节数据, 只会生成一个block, 一个block含有多个piece
     *
     */
    typedef void *raptorq_t;

    const char *raptorq_version();

    /**
     * @brief 创建一个编码器
     *
     * @param piece_per_block 将这一块数据拆成多少piece
     * @param piece_size 每块piece大小(必须偶数大小)
     * @param repair_piece 生成多少个修复piece
     * @param block_data 数据缓存, 长度为piece_per_block * piece_size
     * @return raptorq_t 成功返回非NULL
     */
    raptorq_t raptorq_create_encode(uint32_t piece_per_block, uint16_t piece_size, uint8_t repair_piece, const void *block_data);

    /**
     * @brief 创建一个解码器
     *
     * @param piece_per_block 一块数据有多少个piece
     * @param piece_size 每个piece大小
     * @return raptorq_t 成功返回非NULL
     */
    raptorq_t raptorq_create_decode(uint32_t piece_per_block, uint16_t piece_size);

    /**
     * @brief 预处理
     *
     * @param raptor_handle 句柄
     * @param threads 线程个数
     * @param background 是否阻塞当前线程
     */
    void raptorq_precompute(raptorq_t raptor_handle, uint8_t threads, bool background);

    /**
     * @brief 编码数据, 如果没有调用raptorq_precompute, 则会阻塞直到计算完
     *
     * @param raptor_handle 编码器句柄
     * @param encode_data 编码后的数据存放位置, 至少提供(piece_per_block + repair_piece)个piece_size大小的内存块
     * @param id_vec 存放ID, id和encode_data一一对应, 数组容量至少(piece_per_block + repair_piece)
     * @return true 成功
     * @return false 失败
     */
    bool raptorq_encode(raptorq_t raptor_handle, void *encode_data[], uint32_t piece_id[]);

    /**
     * @brief 向解码器填充piece数据
     *
     * @param raptor_handle 解码器句柄
     * @param piece_data piece数据(大小应为piece_size)
     * @param piece_id piece数据对应的ID(如果Id和数据不对应, 虽然能解码出数据, 但是与源数据不符)
     * @return true 成功, 可继续添加数据
     * @return false 失败, 可尝试调用解码
     */
    bool raptorq_decode_feed_piece(raptorq_t raptor_handle, const void *piece_data, uint32_t piece_id);

    /**
     * @brief 解码数据, 如果没有调用raptorq_precompute, 则会阻塞直到计算完
     *
     * @param raptor_handle 解码器句柄
     * @param decode_data 解码数据存放缓存, 应该提供 piece_per_block * piece_size 大小
     * @param size 缓存大小
     * @return true 成功
     * @return false 失败
     */
    bool raptorq_decode(raptorq_t raptor_handle, void *decode_data, uint32_t size);

    /**
     * @brief 清理编解码器
     *
     * @param raptor_handle 编码器/解码器句柄
     */
    void raptorq_clean(raptorq_t raptor_handle);

#ifdef __cplusplus
}
#endif

#endif // __RAPTOR_Q_H__