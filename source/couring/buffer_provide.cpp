/*************************************************************************
    > File Name: buffer_provide.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时03分29秒
 ************************************************************************/

#include "buffer_provide.h"

#include <utils/exception.h>

#include "uring_service.h"
#include "inner/uring_op_provide_buffer.h"
#include "fiber.h"

namespace eular {
BufferProvide::BufferProvide(const ByteBuffer &buffer, UringService *service, int32_t gid) :
    m_inuse(true),
    m_uringService(service)
{
    if (buffer.size() || service == nullptr) {
        return;
    }

    m_uringOp = std::make_shared<UringOpProvideBuffer>([this] (int32_t status) {
        if (status < 0) {
            throw Exception(String8::format("provide buffer error. %d", status));
        }

        m_inuse = false;
        m_fiber->resume();
    });

    m_bufferId.m_buffer = const_cast<uint8_t *>(buffer.const_data());
    m_bufferId.m_size = buffer.size();
    m_bufferId.m_gid = gid;
    m_bufferId.m_bid = 0;
    m_bufferId.m_isRegistrationBuffer = false;

    int32_t status = service->provideBuffers(m_bufferId.m_buffer, buffer.size(), 1, gid, m_uringOp.get());
    if (status < 0) {
        throw Exception(String8::format("io_uring_register_buffers error. %d", status));
    }
    Fiber::Yeild2Hold();
}

BufferProvide::~BufferProvide()
{
    m_uringService->removeBuffers(1, m_bufferId.m_gid);
}

BufferId *BufferProvide::acquire()
{
    if (m_inuse) {
        return nullptr;
    }

    m_inuse = true;
    return &m_bufferId;
}

void BufferProvide::release(const BufferId *id)
{
    int32_t status = m_uringService->provideBuffers(m_bufferId.m_buffer, m_bufferId.m_size, 1, m_bufferId.m_gid, m_uringOp.get());
    if (status < 0) {
        throw Exception(String8::format("io_uring_register_buffers error. %d", status));
    }
    m_fiber = Fiber::GetThis();
    Fiber::Yeild2Hold();
}

} // namespace eular
