#pragma once

#include <becam/becam.h>
#include <linux/videodev2.h>
#include <vector>

#ifndef _BECAMV4L2_DEVICE_CONFIG_HELPER_H_
#define _BECAMV4L2_DEVICE_CONFIG_HELPER_H_
class Becamv4l2DeviceConfigHelper {
private:
	/**
	 * @brief 设备文件描述句柄（外部管理）
	 */
	int deviceFdHandle;

	/**
	 * @brief 获取设备支持的视频帧帧率列表
	 *
	 * @param frmival [in] 帧率枚举参数
	 * @param outList [out] 帧率信息输出列表引用
	 * @param outListSize [out] 帧率信息输出列表大小引用
	 */
	void getVideoFrameFpsList(v4l2_frmivalenum frmival, VideoFrameFpsInfo*& outList, size_t& outListSize);

	/**
	 * @brief 释放设备支持的视频帧帧率列表
	 *
	 * @param outList [out] 帧率信息输出列表引用
	 * @param outListSize [out] 帧率信息输出列表大小引用
	 */
	static void freeVideoFrameFpsList(VideoFrameFpsInfo*& outList, size_t& outListSize);

	/**
	 * @brief 获取设备支持的视频帧分辨率列表
	 *
	 * @param frmsize [in] 分辨率枚举参数
	 * @param outList [out] 分辨率信息输出列表引用
	 * @param outListSize [out] 分辨率信息输出列表大小引用
	 */
	void getVideoFrameSizeList(v4l2_frmsizeenum frmsize, VideoFrameSizeInfo*& outList, size_t& outListSize);

	/**
	 * @brief 释放设备支持的视频帧分辨率列表
	 *
	 * @param outList [out] 分辨率信息输出列表引用
	 * @param outListSize [out] 分辨率信息输出列表大小引用
	 */
	static void freeVideoFrameSizeList(VideoFrameSizeInfo*& outList, size_t& outListSize);

	/**
	 * @brief 获取设备支持的视频帧格式列表
	 *
	 * @param fmtdesc [in] 格式枚举参数
	 * @param configList [out] 视频帧信息列表引用
	 */
	void getVideoFrameFormatList(v4l2_fmtdesc fmtdesc, std::vector<VideoFrameInfo>& configList);

	/**
	 * @brief 释放设备支持的视频帧格式列表
	 *
	 * @param outList [out] 格式信息输出列表引用
	 * @param outListSize [out] 格式信息输出列表大小引用
	 */
	static void freeVideoFrameFormatList(VideoFrameInfo*& outList, size_t& outListSize);

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
