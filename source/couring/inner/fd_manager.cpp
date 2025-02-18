/*************************************************************************
    > File Name: fd_manager.cpp
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月17日 星期二 17时52分33秒
 ************************************************************************/

#include "inner/fd_manager.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "hook.h"

#include <log/log.h>
#include "fd_manager.h"

#define LOG_TAG     "FdManager"

namespace eular {
FdContext::FdContext(int32_t fd) :
    _isSysNonblock(false),
    _isUserNonblock(false),
    _fd(fd),
    _type(0),
    _recvTimeoutMs(0),
    _sendTimeoutMs(0)
{
    struct stat fileStat;
    if (-1 == fstat(fd, &fileStat)) {
        LOGE("fstat error. [%d, %s]", errno, strerror(errno));
        return;
    }

    _type = fileStat.st_mode;

    // 设置 socket/file/pipe 为非阻塞
    int32_t flags = fcntl_f(_fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK)) {
        fcntl_f(_fd, F_SETFL, flags | O_NONBLOCK);
    }
    _isSysNonblock = true;
}

void FdContext::setTimeout(int32_t type, uint32_t ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = ms % 1000 * 1000;
    if (socketFile()) {
        int32_t status = setsockopt_f(_fd, SOL_SOCKET, type, &tv, sizeof(timeval));
        if (status != 0) {
            const char *stype = (type == SO_RCVTIMEO ? "SO_RCVTIMEO" : "SO_SNDTIMEO");
            LOGE("setsockopt(%d, SOL_SOCKET, %s) error. [%d, %s]", _fd, stype, errno, strerror(errno));
            return;
        }
    }

    switch (type) {
    case SO_RCVTIMEO:
        _recvTimeoutMs = ms;
        break;
    case SO_SNDTIMEO:
        _sendTimeoutMs = ms;
        break;
    default:
        return;
    }
}

FdContext::SP FdManager::get(int32_t fd, bool needCreate)
{
    if (fd < 0) {
        return nullptr;
    }
    auto it = _contextHashMap.find(fd);
    if (it == _contextHashMap.end()) {
        if (!needCreate) {
            return nullptr;
        }

        it = _contextHashMap.insert(std::make_pair(fd, std::make_shared<FdContext>(fd))).first;
    }

    return it->second;
}

void FdManager::remove(int32_t fd)
{
    _contextHashMap.erase(fd);
}

} // namespace eular
