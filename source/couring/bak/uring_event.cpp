/*************************************************************************
    > File Name: uring_event.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月18日 星期三 17时44分00秒
 ************************************************************************/

#include "uring_event.h"

#include <utils/errors.h>
#include <utils/exception.h>

#include "uring_loop.h"
#include "inner/fd_manager.h"

namespace eular {

UringEvent::UringEvent(int32_t fd, UringLoop *loop) :
    m_fd(fd),
    m_ev(IORING_OP_NOP),
    m_persistent(false),
    m_groupId(-1),
    m_loop(loop),
    m_provideInfo(nullptr),
    m_eventInfo(nullptr)
{
    if (loop == nullptr) {
        throw Exception("invalid param loop");
    }

    m_eventInfo = std::make_shared<UserInfo>();
    m_eventInfo->fd = m_fd;
}

UringEvent::~UringEvent()
{
}

int32_t UringEvent::registerEvent(uint32_t ev, EvnetCB cb)
{
    if (ev == IORING_OP_NOP || cb == nullptr) {
        return Status::INVALID_PARAM;
    }

    switch (ev) {
    case IORING_OP_SENDMSG:
    case IORING_OP_RECVMSG:
    case IORING_OP_RECV:
    case IORING_OP_SEND:
    {
        // NOTE 以上操作只针对套接字或管道
        FdContext::SP context = InstanceFromTLS<FdManager>(TAG_FD_MANAGER)->get(m_fd);
        if (context == nullptr || !(context->socketFile() || context->pipeFile())) {
            return Status::INVALID_PARAM;
        }

        break;
    }

    default:
        return Status::INVALID_PARAM;
    }

    m_eventInfo->event = m_ev;
    m_ev = ev;
    m_cb = cb;
    return Status::OK;
}

int32_t UringEvent::registerEvent(uint32_t ev, EvnetBuferCB cb)
{
    if (ev == IORING_OP_NOP || cb == nullptr) {
        return Status::INVALID_PARAM;
    }

    switch (ev) {
    case IORING_OP_SENDMSG:
    case IORING_OP_RECVMSG:
    case IORING_OP_RECV:
    case IORING_OP_SEND:
    {
        // NOTE 以上操作只针对套接字或管道
        FdContext::SP context = InstanceFromTLS<FdManager>(TAG_FD_MANAGER)->get(m_fd);
        if (context == nullptr || !(context->socketFile() || context->pipeFile())) {
            return Status::INVALID_PARAM;
        }

        break;
    }

    default:
        return Status::INVALID_PARAM;
    }

    m_eventInfo->event = m_ev;
    m_ev = ev;
    m_bufCB = cb;
    return Status::OK;
}

void UringEvent::setBufferSize(uint32_t size)
{
    m_buffer.reserve(size);
}

int32_t UringEvent::provideBuffer(int32_t gid)
{
    if (gid <= 0) {
        return Status::INVALID_PARAM;
    }

    // NOTE io_uring_prep_provide_buffers只支持读事件
    switch (m_ev) {
    case IORING_OP_READV:
    // case IORING_OP_READ_FIXED: // FIXED需要和io_uring_register_buffers配合
    case IORING_OP_RECVMSG:
    case IORING_OP_RECV:
    case IORING_OP_READ:
    case IORING_OP_READ_MULTISHOT:
        break;

    default:
        return Status::INVALID_PARAM;
    }

    struct io_uring_probe *probe = io_uring_get_probe_ring(&m_loop->m_uring);
    if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS)) {
        return Status::INVALID_OPERATION;
    }
    
    io_uring_free_probe(probe);

    struct io_uring_sqe *sqe = io_uring_get_sqe(&m_loop->m_uring);
    if (sqe == nullptr) {
        return false;
    }
    m_provideInfo = std::make_shared<UserInfo>();
    m_provideInfo->event = IORING_OP_PROVIDE_BUFFERS;
    m_provideInfo->fd = m_fd;

    m_groupId = gid;
    io_uring_prep_provide_buffers(sqe, m_buffer.data(), m_buffer.capacity(), 1, gid, 0);
    io_uring_sqe_set_data(sqe, m_provideInfo.get());
    return Status::OK;
}

bool UringEvent::reRegisterEvent()
{
    if (!m_persistent) {
        return true;
    }

    struct io_uring_sqe *sqe = io_uring_get_sqe(&m_loop->m_uring);
    if (sqe == nullptr) {
        return false;
    }

    if (m_groupId > 0) {
        io_uring_prep_rw(m_ev, sqe, m_fd, nullptr, m_buffer.capacity(), 0);
        io_uring_sqe_set_flags(sqe, IOSQE_BUFFER_SELECT);
        sqe->buf_group = m_groupId;
    } else {
        io_uring_prep_rw(m_ev, sqe, m_fd, m_buffer.data(), m_buffer.capacity(), 0);
    }
    io_uring_sqe_set_data(sqe, m_eventInfo.get());

    return true;
}

void UringEvent::onCQECallback(io_uring_cqe *cqe)
{
}

} // namespace eular
