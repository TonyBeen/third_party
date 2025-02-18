/*************************************************************************
    > File Name: uring_op_read.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月24日 星期二 19时38分17秒
 ************************************************************************/

#include "uring_op_read.h"

#include <liburing.h>

namespace eular {
UringOpRead::UringOpRead(ProcessReadCB cb) :
    m_cb(cb)
{
}

void UringOpRead::onCompleted(io_uring_cqe *cqe)
{
    if (m_cb && cqe) {
        if (cqe->res < 0) {
            m_cb(cqe->res, 0);
        } else {
            m_cb(0, cqe->res);
        }
    }
}

} // namespace eular
