/*************************************************************************
    > File Name: buffer_registration.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月20日 星期五 18时01分16秒
 ************************************************************************/

#ifndef __COURING_BUFFER_REGISTRATION_H__
#define __COURING_BUFFER_REGISTRATION_H__

#include <vector>
#include <memory>

#include <liburing.h>

#include <utils/buffer.h>
#include <utils/bitmap.h>
#include <utils/utils.h>

#include "buffer_id.h"

namespace eular {
class UringService;

class BufferRegistration
{
    DISALLOW_COPY_AND_ASSIGN(BufferRegistration);
public:
    using SP  = std::shared_ptr<BufferRegistration>;
    using WP  = std::weak_ptr<BufferRegistration>;
    using Ptr = std::unique_ptr<BufferRegistration>;

    BufferRegistration(const std::vector<ByteBuffer> &bufferVec, UringService *service);
    ~BufferRegistration();

    BufferId *acquire();
    void release(const BufferId *id);

private:
    eular::BitMap           m_bitmap;
    UringService*           m_uringService;
    std::vector<BufferId>   m_bufferIdVec;
};

} // namespace eular

#endif // __COURING_BUFFER_REGISTRATION_H__
