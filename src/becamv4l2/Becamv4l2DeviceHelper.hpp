#pragma once

#include <becam/becam.h>
#include <stddef.h>
#include <string.h>

#ifndef _BECAMV4L2_DEVICE_HELPER_H_
#define _BECAMV4L2_DEVICE_HELPER_H_

/**
 * @brief V4L2 设备助手类
 */
class Becamv4l2DeviceHelper {
private:
	/* data */
public:
	/**
	 * @brief 构造函数
	 */
	Becamv4l2DeviceHelper();

    /**
     * @brief 析构函数
     */
	~Becamv4l2DeviceHelper();

    /**
	 * @brief 获取设备列表
	 *
	 * @param reply [out] 设备信息列表引用
	 * @param replySize [out] 设备信息列表大小引用
	 * @return 状态码
	 */
	static StatusCode GetDeviceList(DeviceInfo*& reply, size_t& replySize);

	/**
	 * @brief 释放已获取的设备列表
	 *
	 * @param input [in && out] 已获取的设备列表引用
	 * @param inputSize [in && out] 已获取的设备列表大小引用
	 */
	static void FreeDeviceList(DeviceInfo*& input, size_t& inputSize);
};

#endif