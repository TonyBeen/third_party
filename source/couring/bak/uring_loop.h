/*************************************************************************
    > File Name: uring_loop.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月18日 星期三 17时44分05秒
 ************************************************************************/

#ifndef __COURING_URING_LOOP_H__
#define __COURING_URING_LOOP_H__

#include <liburing.h>

#include "uring_event.h"

namespace eular {

struct UserInfo {
    int32_t     fd;
    int32_t     event; // io_uring_op
};

class UringLoop
{
    friend class UringEvent;
public:
    UringLoop(uint32_t size, uint32_t flag = 0);
    ~UringLoop();

    

private:
    uint32_t            m_size;
    struct io_uring     m_uring;
};

} // namespace eular

#endif // __COURING_URING_LOOP_H__
