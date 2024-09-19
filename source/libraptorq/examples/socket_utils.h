/*************************************************************************
    > File Name: socket_utils.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月18日 星期三 19时15分11秒
 ************************************************************************/

#ifndef __EXAMPLES_SOCKET_UTILS_H__
#define __EXAMPLES_SOCKET_UTILS_H__

#include <stdint.h>
#include <string>

#include <memory>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MTU_SIZE    1400

#define PIECE_SIZE  1280

#define CLR_RED     "\033[31m" // 红色字
#define CLR_GREEN   "\033[32m" // 绿色字
#define CLR_CLR     "\033[0m"  // 恢复颜色

#define SCOP_CLEANUP(arg, _cleanup) \
    std::shared_ptr<void> cleanup_##arg(nullptr, _cleanup);

enum class State {
    Connected = 0,
    TransferFileBegin,
    TransferFileBlockBegin,
    TransferFileBlockEnd,
    TransferFileEnd,
    Closed
};

enum class CommandEnum {
    Success = 0xF,
    Connect,
    TransferFileBegin,
    TransferFileBlockBegin,
    TransferFilePiece,
    TransferFileBlockEnd,
    TransferFileEnd,
    Close
};

struct Command
{
    uint16_t cmd;
    uint16_t length;
}; //  __attribute__((packed))

struct CommandTransferFileBegin
{
    uint32_t piece_per_block;
    char file_name[128];
};

struct CommandTransferFilePiece
{
    uint32_t piece_id;
    char piece_buffer[PIECE_SIZE];
};

struct CommandTransferFileEnd
{
    uint32_t crc32;
};

struct FileBlock
{
    uint32_t block_id;
    uint32_t block_size;
};

class SocketUtils
{
public:
    static int32_t Create(const std::string &host, uint16_t port = 0);

    static bool SetRecvTimeout(int32_t sock, uint16_t time = 1000);
};

#endif // __EXAMPLES_SOCKET_UTILS_H__
