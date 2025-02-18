/*************************************************************************
    > File Name: uring_service.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月22日 星期日 13时29分33秒
 ************************************************************************/

#ifndef __COURING_URING_SERVICE_H__
#define __COURING_URING_SERVICE_H__

#include <stdint.h>

#include <memory>

#include <utils/utils.h>
// #include <liburing.h>

#include "scheduler.h"

namespace eular {
class UringService : public Scheduler
{
    DISALLOW_COPY_AND_ASSIGN(UringService);
public:
    using SP  = std::shared_ptr<UringService>;
    using WP  = std::weak_ptr<UringService>;
    using Ptr = std::unique_ptr<UringService>;

    UringService();
    UringService(uint32_t entries, struct io_uring_params *uringParams = nullptr);
    ~UringService();

    int32_t registerBuffers(const struct iovec *iovecs, size_t size);
    void    unregisterBuffers();

    int32_t provideBuffers(const void *buffer, size_t size, int32_t count, int32_t gid, void *user);
    void    removeBuffers(int32_t count, int32_t gid);

    struct io_uring *handle() const { return m_uringHandle.get(); }

protected:
    void idle() override;

private:
    std::unique_ptr<struct io_uring> m_uringHandle;
};

} // namespace eular

#endif // __COURING_URING_SERVICE_H__
