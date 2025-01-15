#pragma once

#include <becam/becam.h>
#include <stddef.h>
#include <string.h>
#include <mutex>

#ifndef _BECAMV4L2_DEVICE_HELPER_H_
#define _BECAMV4L2_DEVICE_HELPER_H_

/**
 * @brief V4L2 设备助手类
 */
class Becamv4l2DeviceHelper {
private:
	// 互斥锁
	std::mutex mtx;
	// 已激活的设备（内部管理）
	int activatedDevice = -1;

	/**
	 * @brief 处理设备名称
	 *
	 * @param deviceName [in] 设备名称
	 * @return 处理后的设备名称
	 */
	static std::string TrimDeviceName(const std::string& deviceName);

	/**
	 * @brief 检查设备是否支持视频捕获能力
	 *
	 * @param devicePath [in] 设备路径
	 * @param deviceName [out] 设备名称（仅在设备支持视频捕获能力时返回）
	 * @return 是否支持视频捕获能力
	 */
	static bool IsVideoCaptureDevice(const std::string& devicePath, std::string& deviceName);

	/**
	 * @brief 释放当前设备
	 */
	void ReleaseCurrentDevice();

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

	/**
	 * @brief 激活指定设备
	 *
	 * @param devicePath [in] 设备符号链接地址
	 * @return 状态码
	 */
	StatusCode ActivateDevice(const std::string& devicePath);

	/**
	 * @brief 获取当前设备支持的配置列表
	 *
	 * @param reply [out] 视频帧信息列表引用
	 * @param replySize [out] 视频帧信息列表大小引用
	 * @return 状态码
	 */
	StatusCode GetCurrentDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize);

	/**
	 * @brief 释放已获取的设备支持的配置列表
	 *
	 * @param input [in && out] 已获取的视频帧信息列表引用
	 * @param inputSize [in && out] 已获取的视频帧信息列表大小引用
	 */
	static void FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize);
};

#endif