#pragma once

#include <becam/becam.h>

#ifndef _BECAMV4L2_DEVICE_CONFIG_HELPER_H_
#define _BECAMV4L2_DEVICE_CONFIG_HELPER_H_
class Becamv4l2DeviceConfigHelper {
private:
	/**
	 * @brief 设备文件描述句柄（外部管理）
	 */
	int deviceFdHandle;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param fd [in] 设备文件描述句柄（外部管理）
	 */
	Becamv4l2DeviceConfigHelper(const int fd);

	/**
	 * @brief 析构函数
	 */
	~Becamv4l2DeviceConfigHelper();

	/**
	 * @brief 获取设备支持的配置列表
	 *
	 * @param reply [out] 视频帧信息列表引用
	 * @param replySize [out] 视频帧信息列表大小引用
	 * @return 状态码
	 */
	StatusCode GetDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize);

	/**
	 * @brief 释放已获取的设备支持的配置列表
	 *
	 * @param input [in && out] 已获取的视频帧信息列表引用
	 * @param inputSize [in && out] 已获取的视频帧信息列表大小引用
	 */
	static void FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize);
};

#endif
