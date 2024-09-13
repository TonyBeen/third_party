/*************************************************************************
    > File Name: raptorq.c
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月13日 星期五 09时50分09秒
 ************************************************************************/

#include "raptorq/raptorq.h"

#include <stdio.h>

#include "cRaptorQ.h"

#define RAPTOR_ENCODE   1
#define RAPTOR_DECODE   2

#define ID_SIZE     4

typedef struct _Raptorq
{
    uint32_t        type;
    RaptorQ_ptr*    raptorq_ptr;
} SRaptorq;

raptorq_t raptorq_encode_create(uint32_t piece_per_block, uint16_t piece_size, uint8_t repair_piece, const void *block_data)
{
    if (block_data == NULL) {
        return NULL;
    }

    // 小于最小要求或者为奇数
    if (piece_size < MIN_PIECE_SIZE || piece_size & 0x01) {
        return NULL;
    }

    SRaptorq *encoder = (SRaptorq *)malloc(sizeof(SRaptorq));
    if (encoder == NULL) {
        return NULL;
    }

    void *data = const_cast<void *>(block_data);
    uint64_t data_size = piece_per_block * piece_size;
    uint16_t symbol_size = (piece_size - ID_SIZE); // 头四个字节存储 id(小端字节序)
    uint16_t min_subsymbol_size = symbol_size / 2;
    size_t max_memory = static_cast<size_t>(data_size);
    encoder->type = RAPTOR_ENCODE;
    encoder->raptorq_ptr = RaptorQ_Enc(ENC_8, data, data_size, min_subsymbol_size, symbol_size, max_memory);

    return encoder;
}

raptorq_t raptorq_decode_create(uint32_t piece_per_block, uint16_t piece_size)
{
    UNUSED(piece_per_block);
    UNUSED(piece_size);

    return NULL;
}

void raptorq_precompute(raptorq_t raptor_handle, uint8_t threads, BOOL background)
{
    UNUSED(raptor_handle);
    UNUSED(threads);
    UNUSED(background);
}

BOOL raptorq_encode(raptorq_t raptor_handle, void *encode_data[], uint32_t size)
{
    UNUSED(raptor_handle);
    UNUSED(encode_data);
    UNUSED(size);

    return FALSE;
}

BOOL raptorq_decode_feed_piece(raptorq_t raptor_handle, const void *piece_data)
{
    UNUSED(raptor_handle);
    UNUSED(piece_data);

    return FALSE;
}

BOOL raptorq_decode(raptorq_t raptor_handle, void *decode_data, uint32_t size)
{
    UNUSED(raptor_handle);
    UNUSED(decode_data);
    UNUSED(size);

    return FALSE;
}

void raptorq_clean(raptorq_t raptor_handle)
{
    UNUSED(raptor_handle);
}
