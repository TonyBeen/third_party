/*************************************************************************
    > File Name: buffer_base.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时13分21秒
 ************************************************************************/

#ifndef __COURING_BUFFER_BASE_H__
#define __COURING_BUFFER_BASE_H__

#include <memory>

#include <utils/utils.h>

#include "buffer_id.h"

namespace eular {

class BufferBase
{
    DISALLOW_COPY_AND_ASSIGN(BufferBase);
public:
    using SP  = std::shared_ptr<BufferBase>;
    using WP  = std::weak_ptr<BufferBase>;
    using Ptr = std::unique_ptr<BufferBase>;

    BufferBase() = default;
    virtual ~BufferBase() {}

    virtual BufferId *acquire() = 0;
    virtual void release(const BufferId *id) = 0;
};

} // namespace eular

#endif // __COURING_BUFFER_BASE_H__
