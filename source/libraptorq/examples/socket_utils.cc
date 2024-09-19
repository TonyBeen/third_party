/*************************************************************************
    > File Name: socket_utils.cc
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月18日 星期三 19时15分14秒
 ************************************************************************/

#include "socket_utils.h"

#include <string.h>

int32_t SocketUtils::Create(const std::string &host, uint16_t port)
{
    int32_t udp_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_sock < 0) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(host.c_str());
    local_addr.sin_port = htons(port);

    socklen_t len = sizeof(sockaddr_in);
    int32_t error_code = ::bind(udp_sock, (struct sockaddr *)&local_addr, len);
    if (error_code < 0) {
        perror("bind error");
        return -1;
    }

    getsockname(udp_sock, (sockaddr *)&local_addr, &len);
    printf("socket(%d) bind [%s:%d]\n", udp_sock, inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
    return udp_sock;
}

bool SocketUtils::SetRecvTimeout(int32_t sock, uint16_t time)
{
    struct timeval timeout;
    timeout.tv_sec = time / 1000;
    timeout.tv_usec = time % 1000 * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        return false;
    }

    return true;
}
