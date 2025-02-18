/*************************************************************************
    > File Name: hook.h
    > Author: hsz
    > Brief: hook系统函数
    > Created Time: Mon 21 Mar 2022 04:01:53 PM CST
 ************************************************************************/

#ifndef __COURING_HOOK_H__
#define __COURING_HOOK_H__

#include <stdint.h>
#include <time.h>

#include <utils/utils.h>

namespace eular {
namespace hook {

bool HookEnabled();
void SetHookEnable(bool flag = true);

} // namespace hook
} // namespace eular

EXTERN_C_BEGIN

// control
typedef int (*fcntl_fun)(int fd, int cmd, ...);
extern fcntl_fun fcntl_f;

typedef int (*ioctl_fun)(int d, unsigned long int request, ...);
extern ioctl_fun ioctl_f;

typedef int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
extern getsockopt_fun getsockopt_f;

typedef int (*setsockopt_fun)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
extern setsockopt_fun setsockopt_f;

extern int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms = 0);

EXTERN_C_END

#endif // __COURING_HOOK_H__
