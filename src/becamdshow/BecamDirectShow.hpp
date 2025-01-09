#pragma once

#ifndef _BECAM_DIRECT_SHOW_H_
#define _BECAM_DIRECT_SHOW_H_

#include "BecamOpenedDevice.hpp"
#include <becam/becam.h>
#include <dshow.h>
#include <functional>
#include <iostream>

/**
 * @brief 基于DirectShow实现Becam接口
 *
 */
class BecamDirectShow {
private:
	// 声明互斥锁
	std::mutex mtx;
	// 已打开设备实例
	BecamOpenedDevice* openedDevice = nullptr;

public:
	/**
	 * @brief 构造函数
	 */
	BecamDirectShow();

	/**
	 * @brief 析构函数
	 */
	~BecamDirectShow();

	/**
	 * @brief 获取设备列表
	 *
	 * @param reply [out] 响应参数
	 * @return 状态码
	 */
	StatusCode GetDeviceList(GetDeviceListReply* reply);

	/**
	 * @brief 释放设备列表
	 *
	 * @param input [in] 输入参数
	 */
	void FreeDeviceList(GetDeviceListReply* input);

	/**
	 * @brief 获取设备配置列表
	 *
	 * @param devicePath [in] 设备路径
	 * @param reply [out] 输出参数
	 * @return 状态码
	 */
	StatusCode GetDeviceConfigList(const std::string devicePath, GetDeviceConfigListReply* reply);

	/**
	 * @brief 释放设备配置列表
	 *
	 * @param input [in] 输入参数
	 */
	void FreeDeviceConfigList(GetDeviceConfigListReply* input);

	/**
	 * @brief 打开指定设备
	 *
	 * @param devicePath [in] 设备路径
	 * @param frameInfo [in] 设置的视频帧信息
	 * @return 状态码
	 */
	StatusCode OpenDevice(const std::string devicePath, const VideoFrameInfo* frameInfo);

	/**
	 * @brief 关闭设备
	 */
	void CloseDevice();

	/**
	 * @brief 获取视频帧
	 *
	 * @param data 视频帧流
	 * @param size 视频帧流大小
	 * @return 状态码
	 */
	StatusCode GetFrame(uint8_t** data, size_t* size);

	/**
	 * @brief 释放视频帧
	 *
	 * @param data 视频帧流
	 */
	void FreeFrame(uint8_t** data);
};

#endif
