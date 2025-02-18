/*************************************************************************
    > File Name: uring_op_write.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月24日 星期二 20时13分13秒
 ************************************************************************/

#pragma once

#include "uring_operation.h"

namespace eular {
class UringOpWrite : public UringOpBase
{
public:
    using ProcessWriteCB = std::function<void(int32_t, uint32_t)>; // @1 error @2 write_size

    UringOpWrite(ProcessWriteCB cb);
    ~UringOpWrite() = default;

    void onCompleted(io_uring_cqe *cqe) override;

private:
    ProcessWriteCB  m_cb;
};

} // namespace eular
