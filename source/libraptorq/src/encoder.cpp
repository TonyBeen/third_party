/*************************************************************************
    > File Name: encoder.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月13日 星期五 09时50分25秒
 ************************************************************************/

#include "raptorq/encoder.h"

#include <exception>
#include <stdexcept>

#include "raptorq/raptorq.h"

namespace raptorq {

struct EncoderPrivate {
    uint32_t    piece_per_block = 0;
    uint16_t    piece_size = 0;
    uint8_t     redundancy_piece = 0;
    raptorq_t   raptor_handle = nullptr;

    void reset()
    {
        piece_per_block = 0;
        piece_size = 0;
        redundancy_piece = 0;
        if (raptor_handle != nullptr) {
            raptorq_clean(raptor_handle);
            raptor_handle = nullptr;
        }
    }

    ~EncoderPrivate() {
        reset();
    }
};

Encoder::Encoder() :
    m_encoder(new EncoderPrivate)
{
}

Encoder::Encoder(uint32_t piece_per_block, uint16_t piece_size, uint8_t redundancy_piece, const void *block_data) :
    m_encoder(new EncoderPrivate)
{
    if (!reset(piece_per_block, piece_size, redundancy_piece, block_data)) {
        throw std::runtime_error("raptorq_create_encode error");
    }
}

Encoder::~Encoder()
{
    reset();
}

bool Encoder::reset(uint32_t piece_per_block, uint16_t piece_size, uint8_t redundancy_piece, const void *block_data)
{
    m_encoder->reset();
    if (block_data == nullptr) {
        return true;
    }

    m_encoder->piece_per_block = piece_per_block;
    m_encoder->piece_size = piece_size;
    m_encoder->redundancy_piece = redundancy_piece;
    m_encoder->raptor_handle = raptorq_create_encode(piece_per_block, piece_size, redundancy_piece, block_data);
    return m_encoder->raptor_handle != nullptr;
}

void Encoder::precompute(uint8_t threads)
{
    if (m_encoder->raptor_handle != nullptr) {
        raptorq_precompute(m_encoder->raptor_handle, threads, false);
    }
}

bool Encoder::encode(std::vector<uint32_t> &id_vec, std::vector<void *> &data_vec)
{
    if (m_encoder->raptor_handle == nullptr) {
        return false;
    }

    id_vec.resize(m_encoder->piece_per_block + m_encoder->redundancy_piece, 0);
    data_vec.resize(m_encoder->piece_per_block + m_encoder->redundancy_piece, nullptr);
    for (auto &it : data_vec) {
        it = malloc(m_encoder->piece_size);
    }

    if (!raptorq_encode(m_encoder->raptor_handle, data_vec.data(), id_vec.data())) {
        for (auto &it : data_vec) {
            free(it);
            it = nullptr;
        }
        return false;
    }

    return true;
}

} // namespace raptorq
