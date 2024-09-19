/*************************************************************************
    > File Name: udp_server.cc
    > Author: hsz
    > Brief: 示例程序, 并发为1. (不考虑大小端)
    > Created Time: 2024年09月18日 星期三 19时01分28秒
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include <list>
#include <map>

#include <getopt.h>
#include <unistd.h>

#include "socket_utils.h"
#include "raptorq/decoder.h"
#include "crc32.h"

void PrintHelpMsg(const char *exe)
{
    printf("%s [option]\n", exe);
    printf("\t-p bind port\n");
    printf("\t-h help infomation\n");
}

void SendClose(int32_t sock, const sockaddr_in &remote_host)
{
    Command msg;
    msg.cmd = (uint16_t)CommandEnum::Close;
    sendto(sock, &msg, sizeof(Command), 0, (sockaddr *)&remote_host, sizeof(sockaddr_in));
}

void SendSuccess(int32_t sock, const sockaddr_in &remote_host)
{
    Command msg;
    msg.cmd = (uint16_t)CommandEnum::Success;
    sendto(sock, &msg, sizeof(Command), 0, (sockaddr *)&remote_host, sizeof(sockaddr_in));
}

/// @brief 
/// @param argc 
/// @param argv 
/// @return 
int main(int argc, char **argv)
{
    char c = '\x0';
    uint16_t port = 12000;
    while ((c = getopt(argc, argv, "hp:")) > 0) {
        switch (c) {
        case 'p':
            port = static_cast<uint16_t>(atoi(optarg));
            break;
        default:
            PrintHelpMsg(argv[0]);
            return 0;
        }
    }

    int32_t udp_socket = SocketUtils::Create("0.0.0.0", port);
    if (udp_socket < 0) {
        return 0;
    }

    char buffer[MTU_SIZE] = {0};
    sockaddr_in remote_host;

    // 文件
    FILE *file_handle = nullptr;
    std::map<uint32_t, std::string> piece_map;
    uint32_t piece_per_block = 0;
    uint32_t block_id = 0; // 当前往文件可写的ID
    std::map<uint32_t, std::string> file_block_map;
    uint64_t crc_verification = 0;

    auto resource_clean = [&] () {
        if (file_handle) {
            fclose(file_handle);
            file_handle = nullptr;
        }
        piece_per_block = 0;
        piece_map.clear();
        file_block_map.clear();
        crc_verification = 0;
    };

    State state = State::Closed;
    for(;;) {
        sockaddr_in temp_remote_host;
        socklen_t host_len = sizeof(sockaddr_in);
        int32_t recv_size = ::recvfrom(udp_socket, buffer, MTU_SIZE, 0, (sockaddr *)&temp_remote_host, &host_len);
        if (recv_size < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            perror("recvfrom error");
            break;
        }

        // 0包长
        if (recv_size == 0) {
            continue;
        }

        assert(recv_size >= sizeof(Command));
        Command *command = (Command *)buffer;
        if (command->cmd == (uint16_t)State::Connected && state != State::Closed) { // 存在连接的情况下另一个客户端想连接
            SendClose(udp_socket, temp_remote_host);
            continue;
        }
        remote_host = temp_remote_host;

        switch (static_cast<CommandEnum>(command->cmd)) {
        case CommandEnum::Connect:
            printf("accept client [%s:%d]\n", inet_ntoa(remote_host.sin_addr), ntohs(remote_host.sin_port));
            state = State::Connected;
            SendSuccess(udp_socket, remote_host);
            break;
        case CommandEnum::TransferFileBegin:
        {
            if (state != State::Connected) {
                printf("Command sequence error. Current: %u, Command: TransferFileBegin\n", (uint32_t)state);
                SendClose(udp_socket, remote_host);
                state = State::Closed;
                break;
            }

            state = State::TransferFileBegin;
            CommandTransferFileBegin *file_begin = (CommandTransferFileBegin *)(buffer + sizeof(Command));
            printf("[%s:%d] -> transfer file: %s\n", inet_ntoa(remote_host.sin_addr), ntohs(remote_host.sin_port), file_begin->file_name);
            file_handle = fopen(file_begin->file_name, "w+");
            if (file_handle == nullptr) {
                printf("open [%s] error: %s\n", file_begin->file_name, strerror(errno));
                SendClose(udp_socket, remote_host);
                state = State::Closed;
                break;
            }

            piece_per_block = file_begin->piece_per_block;
            SendSuccess(udp_socket, remote_host);
            break;
        }
        case CommandEnum::TransferFileBlockBegin:
            if (state != State::TransferFileBegin) {
                printf("Command sequence error. Current: %u, Command: TransferFileBlockBegin\n", (uint32_t)state);
                SendClose(udp_socket, remote_host);
                state = State::Closed;
                break;
            }
            state = State::TransferFileBlockBegin;
            piece_map.clear();
            SendSuccess(udp_socket, remote_host);
            break;
        case CommandEnum::TransferFilePiece:
        {
            if (state != State::TransferFileBlockBegin) {
                printf("Command sequence error. Current: %u, Command: TransferFilePiece\n", (uint32_t)state);
                SendClose(udp_socket, remote_host);
                state = State::Closed;
                break;
            }

            CommandTransferFilePiece *file_piece = (CommandTransferFilePiece *)(buffer + sizeof(Command));
            piece_map[file_piece->piece_id] = std::string(file_piece->piece_buffer, PIECE_SIZE);
            SendSuccess(udp_socket, remote_host);
            break;
        }
        case CommandEnum::TransferFileBlockEnd:
        {
            raptorq::Decoder raptorq_decoder(piece_per_block, PIECE_SIZE);
            for (const auto &it : piece_map) {
                if (!raptorq_decoder.feedPiece(it.first, it.second)) {
                    break;
                }
            }
            piece_map.clear();

            raptorq_decoder.precompute(1);
            std::string file_block;
            if (!raptorq_decoder.decode(file_block)) { // 解码失败清理资源
                resource_clean();

                SendClose(udp_socket, remote_host);
                state = State::Closed;
                break;
            }

            FileBlock *st_file_block = (FileBlock *)file_block.c_str();
            if (st_file_block->block_id == block_id) {
                ++block_id;

                const void *block_begin = file_block.c_str() + sizeof(FileBlock);
                uint32_t block_size = st_file_block->block_size;
                fwrite(block_begin, block_size, 1, file_handle);
                crc_verification = crc32(crc_verification, block_begin, block_size);
            } else {
                file_block_map[st_file_block->block_id] = std::move(file_block);
            }

            SendSuccess(udp_socket, remote_host);
            state = State::TransferFileBegin;
            break;
        }
        case CommandEnum::TransferFileEnd:
        {
            for (const auto &it : file_block_map) {
                const std::string &file_block = it.second;
                FileBlock *st_file_block = (FileBlock *)file_block.c_str();
                const void *block_begin = file_block.c_str() + sizeof(FileBlock);
                uint32_t block_size = st_file_block->block_size;
                fwrite(block_begin, block_size, 1, file_handle);
                crc_verification = crc32(crc_verification, block_begin, block_size);
            }

            CommandTransferFileEnd *file_end = (CommandTransferFileEnd *)(buffer + sizeof(Command));
            printf("File CRC32 = %u, Compute CRC32 = %" PRIu64 "\n", file_end->crc32, crc_verification);
            if (file_end->crc32 == crc_verification) {
                printf(CLR_GREEN "CRC verification passed\n" CLR_CLR);
            } else {
                printf(CLR_RED "CRC verification failed\n" CLR_CLR);
            }

            resource_clean();
            state = State::Closed;
            break;
        }
        case CommandEnum::Close:
            resource_clean();
            state = State::Closed;
            break;
        default:
            break;
        }
    }

    if (file_handle != nullptr) {
        fclose(file_handle);
    }
    ::close(udp_socket);
    return 0;
}
