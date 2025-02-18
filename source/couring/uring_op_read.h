/*************************************************************************
    > File Name: uring_op_read.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月24日 星期二 19时38分13秒
 ************************************************************************/

#pragma once

#include "uring_operation.h"

namespace eular {
class UringOpRead : public UringOpBase
{
public:
    using ProcessReadCB = std::function<void(int32_t, uint32_t)>; // @1 error @2 read_size

    UringOpRead(ProcessReadCB cb);
    ~UringOpRead() = default;

    void onCompleted(io_uring_cqe *cqe) override;

private:
    ProcessReadCB   m_cb;
};

} // namespace eular
