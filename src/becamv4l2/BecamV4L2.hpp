#pragma once

#ifndef _BECAM_MV4L2_H_
#define _BECAM_MV4L2_H_

#include "Becamv4l2DeviceHelper.hpp"
#include <becam/becam.h>
#include <mutex>

/**
 * @brief 基于V4L2实现Becam接口
 *
 */
class BecamV4L2 {
private:
	// 声明互斥锁
	std::mutex mtx;
	// 已打开设备实例
	Becamv4l2DeviceHelper* openedDevice = new Becamv4l2DeviceHelper();

public:
	/**
	 * @brief 构造函数
	 */
	BecamV4L2();

	/**
	 * @brief 析构函数
	 */
	~BecamV4L2();

	/**
	 * @brief 获取设备列表
	 *
	 * @param reply [out] 响应参数
	 * @return 状态码
	 */
	StatusCode GetDeviceList(GetDeviceListReply& reply);

	/**
	 * @brief 释放设备列表
	 *
	 * @param input [in] 输入参数
	 */
	void FreeDeviceList(GetDeviceListReply& input);

	/**
	 * @brief 获取设备配置列表
	 *
	 * @param devicePath [in] 设备路径
	 * @param reply [out] 输出参数
	 * @return 状态码
	 */
	StatusCode GetDeviceConfigList(const std::string& devicePath, GetDeviceConfigListReply& reply);

	/**
	 * @brief 释放设备配置列表
	 *
	 * @param input [in] 输入参数
	 */
	void FreeDeviceConfigList(GetDeviceConfigListReply& input);

	/**
	 * @brief 打开指定设备
	 *
	 * @param devicePath [in] 设备路径
	 * @param capInfo [in] 捕获信息
	 * @return 状态码
	 */
	StatusCode OpenDevice(const std::string& devicePath, const VideoFrameCaptureInfo& capInfo);

	/**
	 * @brief 关闭设备
	 */
	void CloseDevice();

	/**
	 * @brief 获取视频帧
	 *
	 * @param data [out] 视频帧流
	 * @param size [out] 视频帧流大小
	 * @return 状态码
	 */
	StatusCode GetFrame(uint8_t*& data, size_t& size);

	/**
	 * @brief 释放视频帧
	 *
	 * @param data [in && out] 视频帧流
	 */
	void FreeFrame(uint8_t*& data);
};

#endif
