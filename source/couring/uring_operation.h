/*************************************************************************
    > File Name: uring_operation.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月22日 星期日 13时35分24秒
 ************************************************************************/

#ifndef __COURING_URING_OPERATION_H__
#define __COURING_URING_OPERATION_H__

#include <memory>
#include <functional>

#include <utils/utils.h>

struct io_uring_cqe;

namespace eular {
class UringOpBase
{
    DISALLOW_COPY_AND_ASSIGN(UringOpBase);
public:
    using SP  = std::shared_ptr<UringOpBase>;
    using WP  = std::weak_ptr<UringOpBase>;
    using Ptr = std::unique_ptr<UringOpBase>;

    UringOpBase() = default;
    virtual ~UringOpBase() = default;

    virtual void onCompleted(io_uring_cqe *cqe) = 0;
};

} // namespace eular

#endif // __COURING_URING_OPERATION_H__
