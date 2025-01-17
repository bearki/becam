#pragma once

#ifndef _XIOCTL_H_
#define _XIOCTL_H_

#include <errno.h>
#include <sys/ioctl.h>

/**
 * @brief 含有对信号中断重试的ioctl调用封装
 */
static int xioctl(int fd, int request, void* arg) {
	int r;
	do {
		r = ioctl(fd, request, arg);
	} while (r == -1 && errno == EINTR); // 被信号中断时重试
	return r;
}

#endif
