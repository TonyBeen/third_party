/*************************************************************************
    > File Name: buffer_provide.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时03分16秒
 ************************************************************************/

#ifndef __COURING_BUFFER_PROVIDE_H__
#define __COURING_BUFFER_PROVIDE_H__

#include <vector>
#include <memory>

#include <liburing.h>

#include <utils/buffer.h>
#include <utils/utils.h>

#include "buffer_id.h"
#include "uring_operation.h"
#include "fiber.h"

namespace eular {
class UringService;

class BufferProvide
{
    DISALLOW_COPY_AND_ASSIGN(BufferProvide);
public:
    using SP  = std::shared_ptr<BufferProvide>;
    using WP  = std::weak_ptr<BufferProvide>;
    using Ptr = std::unique_ptr<BufferProvide>;

    BufferProvide(const ByteBuffer &buffer, UringService *service, int32_t gid);
    ~BufferProvide();

    BufferId *acquire();
    void release(const BufferId *id);

private:
    bool            m_inuse;
    UringService*   m_uringService;
    BufferId        m_bufferId;
    UringOpBase::SP m_uringOp;
    Fiber::SP       m_fiber;
};

} // namespace eular

#endif // __COURING_BUFFER_PROVIDE_H__
