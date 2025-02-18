/*************************************************************************
    > File Name: buffer_registration.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时03分25秒
 ************************************************************************/

#include "buffer_registration.h"

#include <utils/exception.h>

#include "uring_service.h"

namespace eular {
BufferRegistration::BufferRegistration(const std::vector<ByteBuffer> &bufferVec, UringService *service) :
    m_uringService(service)
{
    if (bufferVec.empty() || service == nullptr) {
        return;
    }

    m_bufferIdVec.resize(bufferVec.size());
    m_bitmap.resize(bufferVec.size());

    std::vector<iovec> iovecs(bufferVec.size());

    int32_t i = 0;
    for (auto it = bufferVec.begin(); it != bufferVec.end(); ++it, ++i) {
        BufferId bid;
        bid.m_buffer = const_cast<uint8_t *>(it->const_data());
        bid.m_bid = i;
        bid.m_isRegistrationBuffer = true;
        m_bufferIdVec[i] = bid;

        iovecs[i].iov_base = bid.buffer();
        iovecs[i].iov_len = it->capacity();
    }

    int32_t status = service->registerBuffers(&iovecs[0], iovecs.size());
    if (status < 0) {
        throw Exception(String8::format("io_uring_register_buffers error. %d", status));
    }
}

BufferRegistration::~BufferRegistration()
{
    m_uringService->unregisterBuffers();
}

BufferId *BufferRegistration::acquire()
{
    BufferId *result = nullptr;
    if (m_bitmap.count() >= m_bufferIdVec.size()) {
        return result;
    }

    for (uint32_t i = 0; i < m_bitmap.size(); ++i) {
        if (m_bitmap.at(i)) {
            continue;
        }

        m_bitmap.set(i, true);
        result = &m_bufferIdVec[i];
    }

    return result;
}

void BufferRegistration::release(const BufferId *id)
{
    if (id == nullptr) {
        return;
    }

    if (id->m_isRegistrationBuffer) {
        if (0 <= id->m_bid && id->m_bid < m_bufferIdVec.size()) {
            m_bitmap.set(id->m_bid, false);
        }
    }
}

} // namespace eular
