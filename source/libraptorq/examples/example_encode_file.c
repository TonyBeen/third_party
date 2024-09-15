/*************************************************************************
    > File Name: example_encode_file.c
    > Author: hsz
    > Brief: 读取文件并进行编码, 解码后与源文件进行对比
    > Created Time: 2024年09月15日 星期日 08时19分11秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <getopt.h>
#include <sys/stat.h>

#include "raptorq/raptorq.h"

#define REPAIR_PIECE    2

typedef struct st_encoded_data
{
    void*       data;
    uint32_t    size;
    uint32_t    id;
} encoded_data_t;

void PrintHelpMsg(const char *exe)
{
    printf("%s [option]\n", exe);
    printf("\t-f source file\n");
    printf("\t-p piece per block\n");
    printf("\t-h help infomation\n");
}

uint16_t EncodeFile(const char *file_path, uint32_t piece_per_block, void *encoded_data[], uint32_t *id_vec)
{
    struct stat file_stat;
    if (0 != stat(file_path, &file_stat)) {
        perror("stat error");
        return 0;
    }

    uint32_t file_size = file_stat.st_size;
    uint32_t block_size = file_size;
    if (file_size == 0 || file_size > 1 * 1024 * 1024) { // 超过1M认为大文件
        printf("The file is empty or too large (less than 1Mb)\n");
        return 0;
    }

    uint16_t piece_size = block_size / piece_per_block;
    if (block_size % piece_per_block) { // 存在余数
        piece_size += 1;
    }
    block_size = piece_size * piece_per_block;

    FILE *file_handle = fopen(file_path, "r");
    if (file_handle == NULL) {
        perror("fopen error");
        return 0;
    }

    void *file_data = malloc(block_size);
    assert(file_data != NULL);

    size_t read_size = 0;
    read_size = fread(file_data, block_size, 1, file_handle);
    if (read_size < 0) {
        perror("fopen error");
        goto error_section;
    }

    const uint8_t repair_piece = REPAIR_PIECE;
    raptorq_t raptor_handle = raptorq_create_encode(piece_per_block, piece_size, repair_piece, file_data);
    if (raptor_handle == NULL) {
        goto error_section;
    }

    for (uint32_t i = 0; i < (piece_per_block + repair_piece); ++i) {
        encoded_data[i] = malloc(piece_size);
        assert(encoded_data[i] != NULL);
    }

    raptorq_precompute(raptor_handle, 1, false);
    raptorq_encode(raptor_handle, encoded_data, id_vec);
    raptorq_clean(raptor_handle);

    free(file_data);
    fclose(file_handle);
    return piece_size;

error_section:
    free(file_data);
    fclose(file_handle);
    return 0;
}

int main(int argc, char **argv)
{
    const char *file_path = NULL;
    uint32_t piece_per_block = 16;
    char c = '\x0';
    while ((c = getopt(argc, argv, "hf:p:")) > 0) {
        switch (c) {
        case 'f':
            file_path = optarg;
            break;
        case 'p':
            piece_per_block = atoi(optarg);
            if (piece_per_block <= 0) {
                piece_per_block = 16;
            }
            break;
        case 'h':
            PrintHelpMsg(argv[0]);
            return 0;
        default:
            PrintHelpMsg(argv[0]);
            return 0;
        }
    }

    struct stat file_stat;
    if (0 != stat(file_path, &file_stat)) {
        perror("stat error");
        return 0;
    }
    uint32_t file_size = file_stat.st_size;

    void **encoded_data = (void **)malloc((piece_per_block + REPAIR_PIECE) * sizeof(void *));
    uint32_t *id_vec = (uint32_t *)malloc((piece_per_block + REPAIR_PIECE) * sizeof(uint32_t));
    assert(encoded_data != NULL && id_vec != NULL);
    uint16_t piece_size = EncodeFile(file_path, piece_per_block, encoded_data, id_vec);
    if (!piece_size) {
        return 0;
    }

    srand(time(NULL));
    float loss_rate = 10.0; // 默认丢失率为10%
    uint16_t lost_piece = 0;
    raptorq_t raptorq_handle = raptorq_create_decode(piece_per_block, piece_size);
    for (size_t i = 0; i < (piece_per_block + REPAIR_PIECE); ++i) {
        float dropped = ((float)(rand()) / (float) RAND_MAX) * (float)100.0;
        if (dropped < loss_rate) {
            ++lost_piece;
            continue;
        }

        if (!raptorq_decode_feed_piece(raptorq_handle, encoded_data[i], id_vec[i])) {
            printf("The data is sufficient\n");
            break;
        }
    }
    printf("piece lost: %.2f\n", (lost_piece / (float)piece_per_block));
    raptorq_precompute(raptorq_handle, 1, false);
    void *file_data = malloc(piece_per_block * piece_size);
    assert(file_data != NULL);

    if (!raptorq_decode(raptorq_handle, file_data, piece_per_block * piece_size)) {
        printf("Failed to parse data\n");
        goto free_flag;
    }

    FILE *file_handle = fopen("other.dat", "w+");
    assert(file_handle);
    fwrite(file_data, file_size, 1, file_handle);
    fclose(file_handle);

    raptorq_clean(raptorq_handle);

free_flag:
    for (uint32_t i = 0; i < (piece_per_block + REPAIR_PIECE); ++i) {
        free(encoded_data[i]);
    }
    free(encoded_data);
    free(id_vec);
    free(file_data);
    return 0;
}
