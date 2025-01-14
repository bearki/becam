#pragma once

#include <becam/becam.h>
#include <map>
#include <mfapi.h>
#include <mfidl.h>
#include <pkg/SafeRelease.hpp>

#ifndef _BECAMMF_DEVICE_CONFIG_HELPER_H_
#define _BECAMMF_DEVICE_CONFIG_HELPER_H_

/**
 * @brief Media Foundation 设备配置信息助手类
 */
class BecammfDeviceConfigHelper {
private:
	/**
	 * @brief 设备源
	 * @note 该资源由外部管理
	 */
	IMFMediaSource* pSource = nullptr;
	// 演示文稿描述符（内部管理）
	IMFPresentationDescriptor* pPD = nullptr;
	// 视频流的流描述符（内部管理）
	IMFStreamDescriptor* pSD = nullptr;
	// 媒体类型处理器（内部管理）
	IMFMediaTypeHandler* pHandler = nullptr;

	/**
	 * @brief 释放当前资源
	 */
	void ReleaseCurrent();

public:
	/**
	 * @brief 构造函数
	 *
	 * @param pSource [in] 设备源（该资源由外部管理）
	 */
	BecammfDeviceConfigHelper(IMFMediaSource* pSource);

	/**
	 * @brief 析构函数
	 */
	~BecammfDeviceConfigHelper();

	/**
	 * @brief 转换视频帧格式的类型：FOURCC -> GUID
	 *
	 * @param fmt FOURCC表示的帧格式
	 * @return GUID表示的帧格式
	 */
	static GUID FourccToGuid(FOURCC fmt);

	/**
	 * @brief 转换视频帧格式的类型：GUID -> FOURCC
	 *
	 * @param fmt GUID表示的帧格式
	 * @return FOURCC表示的帧格式
	 */
	static FOURCC GuidToFourcc(GUID fmt);

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
