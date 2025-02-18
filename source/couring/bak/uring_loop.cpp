/*************************************************************************
    > File Name: uring_loop.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月18日 星期三 17时44分07秒
 ************************************************************************/

#include "uring_loop.h"

#include <utils/exception.h>

namespace eular {
UringLoop::UringLoop(uint32_t size, uint32_t flag) :
    m_size(size)
{
    int32_t status = io_uring_queue_init(size, &m_uring, flag);
    if (status < 0) {
        throw Exception("io_uring_queue_init error.");
    }
}

UringLoop::~UringLoop()
{
    io_uring_queue_exit(&m_uring);
}

} // namespace eular
