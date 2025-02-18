/*************************************************************************
    > File Name: user_data.h
    > Author: hsz
    > Brief:
    > Created Time: 2024年12月23日 星期一 19时40分24秒
 ************************************************************************/

#ifndef __COURING_USER_INFO_H__
#define __COURING_USER_INFO_H__

#include <stdint.h>

// TODO 使用fd无法区别出同一个fd的读写操作
struct user_data {
    int32_t     fd;
    int32_t     event; // io_uring_op
};

#endif // __COURING_USER_INFO_H__
