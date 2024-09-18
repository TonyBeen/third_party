/*************************************************************************
    > File Name: decoder.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月13日 星期五 09时50分32秒
 ************************************************************************/

#include "raptorq/decoder.h"

#include <exception>

#include "raptorq/raptorq.h"

namespace raptorq {
struct DecoderPrivate
{
    uint32_t    piece_per_block = 0;
    uint16_t    piece_size = 0;
    raptorq_t   raptor_handle = nullptr;

    void reset()
    {
        piece_per_block = 0;
        piece_size = 0;
        if (raptor_handle != nullptr) {
            raptorq_clean(raptor_handle);
            raptor_handle = nullptr;
        }
    }

    ~DecoderPrivate() {
        reset();
    }
};

Decoder::Decoder() :
    m_decoder(new DecoderPrivate)
{
}

Decoder::Decoder(uint32_t piece_per_block, uint16_t piece_size) :
    m_decoder(new DecoderPrivate)
{
    m_decoder->piece_per_block = piece_per_block;
    m_decoder->piece_size = piece_size;
    m_decoder->raptor_handle = raptorq_create_decode(piece_per_block, piece_size);
}

Decoder::~Decoder()
{
    reset();
}

bool Decoder::reset(uint32_t piece_per_block, uint16_t piece_size)
{
    m_decoder->reset();
    if (piece_size == 0) {
        return true;
    }

    m_decoder->piece_per_block = piece_per_block;
    m_decoder->piece_size = piece_size;
    m_decoder->raptor_handle = raptorq_create_decode(piece_per_block, piece_size);
    return m_decoder->raptor_handle != nullptr;
}

bool Decoder::feedPiece(const void *data, uint32_t id)
{
    if (data == nullptr || m_decoder->raptor_handle == nullptr) {
        return false;
    }

    return raptorq_decode_feed_piece(m_decoder->raptor_handle, data, id);
}

bool Decoder::feedPiece(const std::string &data, uint32_t id)
{
    return feedPiece(data.data(), id);
}

void Decoder::precompute(uint8_t threads)
{
    if (m_decoder->raptor_handle != nullptr) {
        raptorq_precompute(m_decoder->raptor_handle, threads, false);
    }
}

bool Decoder::decode(std::string &source_data)
{
    if (m_decoder->raptor_handle == nullptr) {
        return false;
    }

    source_data.resize(m_decoder->piece_per_block * m_decoder->piece_size);
    void *data_buffer = (void *)source_data.data();
    uint32_t piece_per_block = m_decoder->piece_per_block;
    uint16_t piece_size = m_decoder->piece_size;
    return raptorq_decode(m_decoder->raptor_handle, data_buffer, piece_per_block * piece_size);
}

} // namespace raptorq
