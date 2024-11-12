/*************************************************************************
    > File Name: raptorq.c
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月13日 星期五 09时50分09秒
 ************************************************************************/

#include "raptorq/raptorq.h"

#include <stdio.h>
#include <assert.h>

#include "cRaptorQ.h"
#include "version.h"

#define RAPTOR_ENCODE   1
#define RAPTOR_DECODE   2

typedef struct _Raptorq
{
    uint32_t        type;
    uint32_t        piece_per_block;
    uint16_t        piece_size;
    uint8_t         repair_piece;
    RaptorQ_ptr*    raptorq_ptr;
} SRaptorq;

const char *raptorq_version()
{
    return RAPTORQ_VERSION;
}

raptorq_t raptorq_create_encode(uint32_t piece_per_block, uint16_t piece_size, uint8_t repair_piece, const void *block_data)
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
    uint16_t symbol_size = piece_size;
    uint16_t min_subsymbol_size = symbol_size / 2;
    size_t max_memory = static_cast<size_t>(data_size);
    encoder->type = RAPTOR_ENCODE;
    encoder->piece_per_block = piece_per_block;
    encoder->piece_size = piece_size;
    encoder->repair_piece = repair_piece;
    encoder->raptorq_ptr = RaptorQ_Enc(ENC_8, data, data_size, min_subsymbol_size, symbol_size, max_memory);
    if (encoder->raptorq_ptr == NULL) {
        free(encoder);
        encoder = NULL;
    }

    return encoder;
}

raptorq_t raptorq_create_decode(uint32_t piece_per_block, uint16_t piece_size)
{
    uint64_t data_size = piece_per_block * piece_size;
    uint16_t symbol_size = piece_size;
    uint16_t min_subsymbol_size = symbol_size / 2;
    size_t max_memory = static_cast<size_t>(data_size);
    RaptorQ_ptr *encoder = RaptorQ_Enc(ENC_8, NULL, data_size, min_subsymbol_size, symbol_size, max_memory);
    if (encoder == NULL) {
        return NULL;
    }
    uint32_t oti_scheme = RaptorQ_OTI_Scheme(encoder);
    uint64_t oti_common = RaptorQ_OTI_Common(encoder);
    RaptorQ_free(&encoder);

    SRaptorq *decoder = (SRaptorq *)malloc(sizeof(SRaptorq));
    if (decoder == NULL) {
        return NULL;
    }

    decoder->type = RAPTOR_DECODE;
    decoder->piece_per_block = piece_per_block;
    decoder->piece_size = piece_size;
    decoder->raptorq_ptr = RaptorQ_Dec(DEC_8, oti_common, oti_scheme);
    if (decoder->raptorq_ptr == NULL) {
        free(decoder);
        decoder = NULL;
    }

    return decoder;
}

void raptorq_precompute(raptorq_t raptor_handle, uint8_t threads, bool background)
{
    SRaptorq *raptorq = (SRaptorq *)raptor_handle;
    if (raptorq != NULL) {
        RaptorQ_precompute(raptorq->raptorq_ptr, threads, static_cast<bool>(background));
    }
}

bool raptorq_encode(raptorq_t raptor_handle, void *encode_data[], uint32_t piece_id[])
{
    SRaptorq *raptorq = (SRaptorq *)raptor_handle;
    if (raptorq == NULL || encode_data == NULL || piece_id == NULL) {
        return false;
    }

    if (raptorq->type != RAPTOR_ENCODE) {
        return false;
    }

    uint8_t blocks = RaptorQ_blocks(raptorq->raptorq_ptr);
    assert(1 == blocks);
    if (blocks != 1) {
        return false;
    }

    // 每块 symbol 大小
    uint32_t symbol_size = RaptorQ_symbol_size(raptorq->raptorq_ptr);
    assert(symbol_size == raptorq->piece_size);
    if (symbol_size != raptorq->piece_size) {
        return false;
    }

    uint8_t sbn = blocks - 1;
    // symbol 个数
    uint32_t src_symbols = RaptorQ_symbols(raptorq->raptorq_ptr, sbn);
    assert(src_symbols == raptorq->piece_per_block);
    if (src_symbols != raptorq->piece_per_block) {
        return false;
    }

    uint32_t max_repair_symbol = RaptorQ_max_repair(raptorq->raptorq_ptr, sbn);
    if (max_repair_symbol < raptorq->repair_piece) {
        return false;
    }

    uint32_t i = 0;
    for (uint32_t source = 0; source < (src_symbols + raptorq->repair_piece); ++source, ++i) {
        uint32_t esi = source;
        piece_id[i] = RaptorQ_id(esi, sbn);
        void *date_temp = encode_data[i];
        if (date_temp == NULL) {
            return false;
        }

        uint64_t written = RaptorQ_encode(raptorq->raptorq_ptr, &date_temp, symbol_size, esi, sbn);
        if (written != symbol_size) {
            return false;
        }
    }

    return true;
}

bool raptorq_decode_feed_piece(raptorq_t raptor_handle, const void *piece_data, uint32_t piece_id)
{
    SRaptorq *raptorq = (SRaptorq *)raptor_handle;
    if (raptorq == NULL || raptorq->type != RAPTOR_DECODE) {
        return false;
    }

    uint16_t piece_size = RaptorQ_symbol_size(raptorq->raptorq_ptr);
    assert(piece_size == raptorq->piece_size);
    if (piece_size != raptorq->piece_size) {
        return false;
    }

    void *data_temp = const_cast<void *>(piece_data);
    return RaptorQ_add_symbol_id(raptorq->raptorq_ptr, &data_temp, raptorq->piece_size, piece_id);
}

bool raptorq_decode(raptorq_t raptor_handle, void *decode_data, uint32_t size)
{
    SRaptorq *raptorq = (SRaptorq *)raptor_handle;
    if (raptorq == NULL || raptorq->type != RAPTOR_DECODE) {
        return false;
    }

    uint64_t decode_size = RaptorQ_bytes(raptorq->raptorq_ptr);
    if (decode_size > size) {
        return false;
    }

    uint64_t written = RaptorQ_decode(raptorq->raptorq_ptr, &decode_data, decode_size);
    if (written != decode_size) {
        return false;
    }

    return true;
}

void raptorq_clean(raptorq_t raptor_handle)
{
    SRaptorq *raptorq = (SRaptorq *)raptor_handle;
    if (raptorq == NULL) {
        return;
    }

    RaptorQ_free(&raptorq->raptorq_ptr);
    free(raptorq);
}
