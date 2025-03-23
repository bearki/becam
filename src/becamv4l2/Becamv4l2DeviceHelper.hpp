#pragma once

#include <becam/becam.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <mutex>
#include <stddef.h>
#include <string.h>

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

	// 用户缓冲区数量
	static const int USER_BUFFER_COUNT = 3;

	/**
	 * @brief 单平面捕获用户缓冲区类型
	 */
	struct SingleplaneCaptureUserBuffer {
		size_t length; // 缓冲区长度
		void* address; // 缓冲区地址
	};

	/**
	 * @brief 多平面捕获用户缓冲区类型
	 */
	struct MultiplaneCaptureUserBuffer {
		size_t planeNum;										 // 平面数量
		SingleplaneCaptureUserBuffer planeBuf[VIDEO_MAX_PLANES]; // 平面缓冲区
	};

	// 捕获缓冲区类型（默认为单平面）
	v4l2_buf_type capBufType = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// 单平面捕获缓冲区
	SingleplaneCaptureUserBuffer spCapBufList[Becamv4l2DeviceHelper::USER_BUFFER_COUNT] = {0};
	// 多平面捕获缓冲区
	MultiplaneCaptureUserBuffer mpCapBufList[Becamv4l2DeviceHelper::USER_BUFFER_COUNT] = {0};

	// 是否已经开始取流
	bool streamON = false;

	/**
	 * @brief 处理设备名称
	 *
	 * @param deviceName [in] 设备名称
	 * @return 处理后的设备名称
	 */
	static std::string trimDeviceName(const std::string& deviceName);

	/**
	 * @brief 检查设备是否支持视频捕获能力
	 *
	 * @param devicePath [in] 设备路径
	 * @param deviceName [out] 设备名称（仅在设备支持视频捕获能力时返回）
	 * @return 是否支持视频捕获能力
	 */
	static bool isVideoCaptureDevice(const std::string& devicePath, std::string& deviceName);

	/**
	 * @brief 关闭当前设备
	 */
	void closeCurrentDevice();

	/**
	 * @brief 停止当前设备取流
	 */
	void stopCurrentDeviceStreaming();

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
	 * @param oflags [in] 设备激活模式（只读模式 或 读写模式）
	 * @return 状态码
	 */
	StatusCode ActivateDevice(const std::string& devicePath, const int oflags = O_RDONLY);

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

	/**
	 * @brief 激活设备取流
	 *
	 * @param capInfo [in] 捕获信息
	 * @return 状态码
	 */
	StatusCode ActivateDeviceStreaming(const VideoFrameCaptureInfo& capInfo);

	/**
	 * @brief 关闭设备
	 */
	void CloseDevice();

	/**
	 * @brief 获取视频帧
	 *
	 * @param reply [out] 视频帧数据引用
	 * @param replySize [out] 视频帧数据大小引用
	 * @return 状态码
	 */
	StatusCode GetFrame(uint8_t*& reply, size_t& replySize);

	/**
	 * @brief 释放已获取的视频帧
	 *
	 * @param input [in && out] 已获取的视频帧数据引用
	 */
	static void FreeFrame(uint8_t*& reply);
};

#endif