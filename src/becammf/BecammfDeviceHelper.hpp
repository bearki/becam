#pragma once

#include <becam/becam.h>
#include <mfidl.h>
#include <mutex>
#include <pkg/SafeRelease.hpp>
#include <string>

#ifndef _BECAMMF_DEVICE_HELPER_H_
#define _BECAMMF_DEVICE_HELPER_H_

/**
 * Media Foundation设备助手类
 */
class BecammfDeviceHelper {
private:
	// 互斥锁
	std::mutex mtx;
	// COM库初始化结果
	HRESULT _ComInitResult;
	// Media Foundation库初始化结果
	HRESULT _MFStartupResult;
	// 已激活的设备（内部管理）
	IMFMediaSource* activatedDevice = nullptr;

	/**
	 * @brief 释放当前设备
	 */
	void ReleaseCurrent();

public:
	/**
	 * @brief 构造函数
	 */
	BecammfDeviceHelper();

	/**
	 * @brief 析构函数
	 */
	~BecammfDeviceHelper();

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
	StatusCode ActivateDevice(std::string devicePath);

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
