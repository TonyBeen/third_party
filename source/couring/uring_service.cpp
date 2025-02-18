/*************************************************************************
    > File Name: uring_service.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月22日 星期日 13时29分37秒
 ************************************************************************/

#include "uring_service.h"

#include <liburing.h>

#include <utils/exception.h>
#include <utils/string8.h>
#include <utils/errors.h>

namespace eular {
UringService::UringService() :
    m_uringHandle(new io_uring())
{
    uint32_t entries = 1024;
    int32_t status = io_uring_queue_init(entries, m_uringHandle.get(), 0);
    if (status != 0) {
        throw Exception(String8::format("io_uring_queue_init error. %d", status));
    }

}

UringService::UringService(uint32_t entries, io_uring_params *uringParams) :
    m_uringHandle(new io_uring())
{
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    if (uringParams == nullptr) { // NOTE io_uring_queue_init_params不校验是否为空, 传递null会导致段错误
        uringParams = &params;
    }

    int32_t status = io_uring_queue_init_params(2048, m_uringHandle.get(), uringParams);
    if (status != 0) {
        throw Exception(String8::format("io_uring_queue_init_params error. %d", status));
    }
}

eular::UringService::~UringService()
{
    io_uring_queue_exit(m_uringHandle.get());
}

int32_t UringService::registerBuffers(const struct iovec *iovecs, size_t size)
{
    int32_t status = ::io_uring_register_buffers(m_uringHandle.get(), iovecs, size);
    return status;
}

void UringService::unregisterBuffers()
{
    (void)::io_uring_unregister_buffers(m_uringHandle.get());
}

int32_t UringService::provideBuffers(const void *buffer, size_t size, int32_t count, int32_t gid, void *user)
{
    struct io_uring_probe *probe;
    probe = io_uring_get_probe_ring(m_uringHandle.get());
    if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS)) {
        return Status::NOT_SUPPORT;
    }
    io_uring_free_probe(probe);

    struct io_uring_sqe *sqe = io_uring_get_sqe(m_uringHandle.get());
    if (sqe == nullptr) {
        return Status::NO_MORE_ITEM;
    }

    io_uring_prep_provide_buffers(sqe, const_cast<void *>(buffer), size, count, gid, 0);
    sqe->user_data = (__u64)user;
    return 0;
}

void UringService::removeBuffers(int32_t count, int32_t gid)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(m_uringHandle.get());
    if (sqe == nullptr) {
        throw Exception("io_uring_get_sqe return null");
    }

    io_uring_prep_remove_buffers(sqe, count, gid);
    sqe->user_data = 0;
}

void UringService::idle()
{
}

} // namespace eular