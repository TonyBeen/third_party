/*************************************************************************
    > File Name: udp_client.cc
    > Author: hsz
    > Brief:
    > Created Time: 2024年09月18日 星期三 19时01分58秒
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <list>
#include <map>

#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "socket_utils.h"
#include "raptorq/encoder.h"
#include "crc32.h"

const char *ch_remote_host = nullptr;
uint16_t port = 0;

void PrintHelpMsg(const char *exe)
{
    printf("%s [option]\n", exe);
    printf("\t-f file send\n");
    printf("\t-p bind port\n");
    printf("\t-r server address\n");
    printf("\t-P piece per block\n");
    printf("\t-R repair piece count\n");
    printf("\t-h help infomation\n");
}

void SendCommandAndWaitResponse(CommandEnum cmd, const void *cmd_buffer, uint32_t size, int32_t udp_socket, const sockaddr_in &server_host)
{
    sockaddr_in remote_host;
    socklen_t host_len = sizeof(sockaddr_in);
    char buffer[MTU_SIZE] = {0};

    switch (cmd) {
    case CommandEnum::Connect:
    case CommandEnum::TransferFileBegin:
    case CommandEnum::TransferFileBlockBegin:
    case CommandEnum::TransferFilePiece:
    case CommandEnum::TransferFileBlockEnd:
    {
        ::sendto(udp_socket, cmd_buffer, size, 0, (sockaddr *)&server_host, sizeof(sockaddr_in));

        int32_t error_code = ::recvfrom(udp_socket, buffer, MTU_SIZE, 0, (sockaddr *)&remote_host, &host_len);
        if (error_code < 0) {
            perror("recvfrom error");
            exit(0);
        }
        Command *command = (Command *)buffer;
        if (command->cmd != (uint16_t)CommandEnum::Success) {
            printf("[%s:%u] remote error: %d\n", ch_remote_host, port, command->cmd);
            exit(0);
        }

        break;
    }
    case CommandEnum::TransferFileEnd:
    case CommandEnum::Close:
        break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    char c = '\x0';
    const char *file_path = nullptr;
    uint16_t piece_per_block = 8;
    uint16_t repair_piece = 2;
    while ((c = getopt(argc, argv, "hf:p:r:P:R:")) > 0) {
        switch (c) {
        case 'f':
            file_path = optarg;
            break;
        case 'p':
            port = static_cast<uint16_t>(atoi(optarg));
            break;
        case 'r':
            ch_remote_host = optarg;
            break;
        case 'P':
            piece_per_block = static_cast<uint16_t>(atoi(optarg));
            break;
        case 'R':
            repair_piece = static_cast<uint16_t>(atoi(optarg));
            break;
        default:
            PrintHelpMsg(argv[0]);
            return 0;
        }
    }

    if (file_path == nullptr) {
        printf("File path not set\n");
        return 0;
    }

    struct stat file_stat;
    int32_t error = stat(file_path, &file_stat);
    if (error != 0) {
        perror("stat failed");
        return 0;
    }

    if (piece_per_block == 0 || repair_piece == 0) {
        return 0;
    }

    if (ch_remote_host == nullptr) {
        printf("Service IP not set\n");
        return 0;
    }

    int32_t udp_socket = SocketUtils::Create("0.0.0.0");
    if (udp_socket < 0) {
        return 0;
    }
    SCOP_CLEANUP(udp_socket, [&udp_socket](void *) {
        ::close(udp_socket);
    });

    SocketUtils::SetRecvTimeout(udp_socket, 3000);

    printf("connecting to service [%s:%u]\n", ch_remote_host, port);

    sockaddr_in server_host;
    server_host.sin_family = AF_INET;
    server_host.sin_addr.s_addr = inet_addr(ch_remote_host);
    server_host.sin_port = htons(port);

    sockaddr_in remote_host;
    socklen_t host_len = sizeof(sockaddr_in);

    // 连接
    {
        Command cmd;
        cmd.cmd = (uint16_t)CommandEnum::Connect;

        SendCommandAndWaitResponse(CommandEnum::Connect, &cmd, sizeof(Command), udp_socket, server_host);
    }

    printf("connected to service [%s:%u]\n", ch_remote_host, port);

    // 发送开始传输文件指令
    {
        char buffer[sizeof(Command) + sizeof(CommandTransferFileBegin)] = {0};

        Command *cmd = (Command *)buffer;
        cmd->cmd = (uint16_t)CommandEnum::TransferFileBegin;
        CommandTransferFileBegin *file_begin = (CommandTransferFileBegin *)(buffer + sizeof(Command));
        file_begin->piece_per_block = piece_per_block;
        const char *index = strrchr(file_path, '/'); // 找到最后一个 / 位置
        if (index == nullptr) {
            index = file_path;
        } else {
            index += 1;
        }
        printf("transfer file: %s\n", index);
        strncpy(file_begin->file_name, index, sizeof(file_begin->file_name) - 1); // 保证结尾的\0

        SendCommandAndWaitResponse(CommandEnum::TransferFileBegin, buffer, sizeof(buffer), udp_socket, server_host);
    }

    // 打开文件并发送
    FILE *file_handle = fopen(file_path, "r");
    assert(file_handle != nullptr);
    SCOP_CLEANUP(file_handle, [&](void *) {
        fclose(file_handle);
    })

    uint32_t file_id = 0;
    uint64_t file_crc = 0;
    std::string file_data;
    file_data.resize(piece_per_block * PIECE_SIZE);
    uint32_t offset = sizeof(FileBlock);
    const uint32_t BUFFER_SIZE = piece_per_block * PIECE_SIZE - offset;

    do {
        // 发送 TransferFileBlockBegin 命令
        FileBlock file_block;
        file_block.block_id = file_id++;
        auto read_size = fread(file_data.data() + offset, 1, BUFFER_SIZE, file_handle);
        if (read_size == 0) {
            break;
        }

        file_block.block_size = read_size;
        memcpy(file_data.data(), &file_block, sizeof(file_block));

        file_crc = crc32(file_crc, file_data.data() + offset, read_size);

        raptorq::Encoder raptorq_encoder(piece_per_block, PIECE_SIZE, repair_piece, file_data.data());
        raptorq_encoder.precompute(1);
        std::vector<uint32_t> piece_id_vec;
        std::vector<void *> piece_data_vec;
        if (!raptorq_encoder.encode(piece_id_vec, piece_data_vec)) {
            printf("raptorq encode error\n");
            break;
        }

        Command cmd;
        cmd.cmd = (uint16_t)CommandEnum::TransferFileBlockBegin;
        SendCommandAndWaitResponse(CommandEnum::TransferFileBlockBegin, &cmd, sizeof(cmd), udp_socket, server_host);
        char buffer[MTU_SIZE] = {0};
        for (uint32_t i = 0; i < piece_id_vec.size(); ++i) {
            Command *command = (Command *)buffer;
            command->cmd = (uint16_t)CommandEnum::TransferFilePiece;
            CommandTransferFilePiece *file_piece = (CommandTransferFilePiece *)(buffer + sizeof(Command));
            file_piece->piece_id = piece_id_vec[i];
            memcpy(file_piece->piece_buffer, piece_data_vec[i], PIECE_SIZE);

            free(piece_data_vec[i]);
            piece_data_vec[i] = nullptr;

            uint32_t size = sizeof(Command) + sizeof(CommandTransferFilePiece);
            SendCommandAndWaitResponse(CommandEnum::TransferFilePiece, buffer, size, udp_socket, server_host);
        }

        cmd.cmd = (uint16_t)CommandEnum::TransferFileBlockEnd;
        SendCommandAndWaitResponse(CommandEnum::TransferFileBlockEnd, &cmd, sizeof(cmd), udp_socket, server_host);
    } while (!feof(file_handle));

    // 发送传输文件结束指令
    {
        char buffer[sizeof(Command) + sizeof(CommandTransferFileEnd)] = {0};

        Command *cmd = (Command *)buffer;
        cmd->cmd = (uint16_t)CommandEnum::TransferFileEnd;
        CommandTransferFileEnd *file_end = (CommandTransferFileEnd *)(buffer + sizeof(Command));
        file_end->crc32 = file_crc;

        ::sendto(udp_socket, buffer, sizeof(buffer), 0, (sockaddr *)&server_host, sizeof(sockaddr_in));
    }

    // 发送关闭命令
    {
        Command cmd;
        cmd.cmd = (uint16_t)CommandEnum::Close;
        ::sendto(udp_socket, &cmd, sizeof(cmd), 0, (sockaddr *)&server_host, sizeof(sockaddr_in));
    }

    return 0;
}