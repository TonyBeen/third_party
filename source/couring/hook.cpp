/*************************************************************************
    > File Name: hook.cpp
    > Author: hsz
    > Brief:
    > Created Time: Mon 21 Mar 2022 04:01:59 PM CST
 ************************************************************************/

#include "hook.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

#include <utils/errors.h>
#include <utils/mutex.h>
#include <utils/thread_local.h>
#include <log/log.h>

#include "inner/fd_manager.h"

#define LOG_TAG "hook"

namespace eular {

static uint32_t gConnectTimeoutMs = 3000;

namespace hook {

static thread_local bool gHookEnabled = false;

bool HookEnabled()
{
    return gHookEnabled;
}

void SetHookEnable(bool flag = true)
{
    gHookEnabled = flag;
}

} // namespace hook
}

EXTERN_C_BEGIN
// sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

// file
typedef int (*open_fun)(const char *pathname, int flags, ...);
extern open_fun open_f;

typedef int (*openat_fun)(int dirfd, const char *pathname, int flags, ...);
extern openat_fun openat_f;

// socket
typedef int (*socket_fun)(int domain, int type, int protocol);
extern socket_fun socket_f;

typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern connect_fun connect_f;

typedef int (*accept_fun)(int s, struct sockaddr *addr, socklen_t *addrlen);
extern accept_fun accept_f;

typedef int (*close_fun)(int fd);
extern close_fun close_f;

// read
typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;

typedef ssize_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
extern readv_fun readv_f;

typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
extern recv_fun recv_f;

typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_fun recvfrom_f;

typedef ssize_t (*recvmsg_fun)(int sockfd, struct msghdr *msg, int flags);
extern recvmsg_fun recvmsg_f;

// write
typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
extern write_fun write_f;

typedef ssize_t (*writev_fun)(int fd, const struct iovec *iov, int iovcnt);
extern writev_fun writev_f;

typedef ssize_t (*send_fun)(int s, const void *msg, size_t len, int flags);
extern send_fun send_f;

typedef ssize_t (*sendto_fun)(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
extern sendto_fun sendto_f;

typedef ssize_t (*sendmsg_fun)(int s, const struct msghdr *msg, int flags);
extern sendmsg_fun sendmsg_f;

EXTERN_C_END

#define HOOK_FUN(XX)    \
    XX(sleep)           \
    XX(usleep)          \
    XX(open)            \
    XX(openat)          \
    XX(socket)          \
    XX(connect)         \
    XX(accept)          \
    XX(close)           \
    XX(read)            \
    XX(readv)           \
    XX(recv)            \
    XX(recvfrom)        \
    XX(recvmsg)         \
    XX(write)           \
    XX(writev)          \
    XX(send)            \
    XX(sendto)          \
    XX(sendmsg)         \
    XX(fcntl)           \
    XX(ioctl)           \
    XX(getsockopt)      \
    XX(setsockopt)      \

struct _HookInitialization {
    _HookInitialization()
    {
        eular::call_once(_flag, [] () {
#define XX(name) name ## _f = (name ## _fun)::dlsym(RTLD_NEXT, #name);
            HOOK_FUN(XX);
#undef XX
        });
    }

    eular::once_flag _flag;
} gHookInitialization;

struct _TimerInfo {
    int32_t cancelled = 0;
};

////////////////////////////////////////////// socket //////////////////////////////////////////////////////

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int type, Args&&... args)
{
    LOGD("%s() fd: %d, %s(%p)\n", __func__, fd, hook_fun_name, fun);
    if (!eular::hook::HookEnabled()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    return Status::OK;
}

EXTERN_C_BEGIN

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

extern unsigned int sleep(unsigned int seconds) {
    if (!eular::hook::HookEnabled()) {
        return sleep_f(seconds);
    }

    return 0;
}

int usleep(useconds_t usec)
{
    if (!eular::hook::HookEnabled()) {
        return usleep_f(usec);
    }

    return 0;
}

int open(const char *pathname, int flags, ...)
{
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list arg;
        va_start(arg, flags);
        mode = va_arg(arg, mode_t);
        va_end(arg);
    }

    if (!eular::hook::HookEnabled()) {
        return open_f(pathname, flags, mode);
    }

    int32_t fd = open_f(pathname, flags, mode);
    if (fd < 0) {
        return fd;
    }

    eular::InstanceFromTLS<eular::FdManager>(TAG_FD_MANAGER)->get(fd, true);
    return fd;
}

int socket(int domain, int type, int protocol) {
    if (!eular::hook::HookEnabled()) {
        return socket_f(domain, type, protocol);
    }

    int fd = socket_f(domain, type, protocol);
    if (fd < 0) {
        return fd;
    }

    eular::InstanceFromTLS<eular::FdManager>(TAG_FD_MANAGER)->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, uint64_t ms)
{
    if (!eular::hook::HookEnabled()) {
        return connect_f(fd, addr, addrlen);
    }

    eular::FdContext::SP context = eular::InstanceFromTLS<eular::FdManager>(TAG_FD_MANAGER)->get(fd);
    if (!context) {
        errno = EBADF;
        return -1;
    }

    if (!context->socketFile()) { // 管道, 串口等
        return connect_f(fd, addr, addrlen);
    }

    if (context->_isUserNonblock) {
        return connect_f(fd, addr, addrlen);
    }

    int32_t code = connect_f(fd, addr, addrlen);
    if (code == 0) {
        return 0;
    } else if (code == -1 && errno != EINPROGRESS) {
        return code;
    }

    // TODO 协程异步连接

    return code;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    uint64_t ms = eular::gConnectTimeoutMs;

    // TODO 通过文件描述符管理获取
    struct timeval timeout;
    socklen_t len = sizeof(timeout);
    int32_t status = getsockopt_f(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);
    if (status == 0) {
        ms = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
    }

    return connect_with_timeout(sockfd, addr, addrlen, ms);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    // int fd = do_io(s, accept_f, "accept", eular::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    // if(fd >= 0) {
    //     eular::FdManager::Get()->get(fd, true);
    // }
    // return fd;
}

int close(int fd)
{
    if (!eular::hook::HookEnabled()) {
        return close_f(fd);
    }

    // eular::FdContext::SP ctxsp = eular::FdManager::Get()->get(fd);
    // if (ctxsp) {
    //     auto iom = eular::IOManager::GetThis();
    //     if (iom) {
    //         iom->cancelAll(fd);
    //     }
    //     eular::FdManager::Get()->del(fd);
    // }
    return close_f(fd);
}

ssize_t read(int fd, void *buf, size_t count)
{
    // return do_io(fd, read_f, "read", eular::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    // return do_io(fd, readv_f, "readv", eular::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    // return do_io(sockfd, recv_f, "recv", eular::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    // return do_io(sockfd, recvfrom_f, "recvfrom", eular::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    // return do_io(sockfd, recvmsg_f, "recvmsg", eular::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    // return do_io(fd, write_f, "write", eular::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    // return do_io(fd, writev_f, "writev", eular::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags)
{
    // return do_io(s, send_f, "send", eular::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
    // return do_io(s, sendto_f, "sendto", eular::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
{
    // return do_io(s, sendmsg_f, "sendmsg", eular::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int fcntl(int fd, int cmd, ...) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                // int arg = va_arg(va, int);
                // va_end(va);
                // eular::FdContext::SP ctx = eular::FdManager::Get()->get(fd);
                // if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                //     return fcntl_f(fd, cmd, arg);
                // }
                // ctx->setUserNonblock(arg & O_NONBLOCK);
                // if(ctx->getSysNonblock()) {
                //     arg |= O_NONBLOCK;
                // } else {
                //     arg &= ~O_NONBLOCK;
                // }
                // return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                // int arg = fcntl_f(fd, cmd);
                // eular::FdContext::SP ctx = eular::FdManager::Get()->get(fd);
                // if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                //     return arg;
                // }
                // if(ctx->getUserNonoblock()) {
                //     return arg | O_NONBLOCK;
                // } else {
                //     return arg & ~O_NONBLOCK;
                // }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

EXTERN_C_END