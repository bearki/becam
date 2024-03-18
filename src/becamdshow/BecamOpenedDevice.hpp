#pragma once

#ifndef _BECAM_OPENED_DEVICE_H_
#define _BECAM_OPENED_DEVICE_H_

#include "BecamAmMediaType.hpp"
#include "BecamSampleGrabberCallback.hpp"
#include <becam/becam.h>
#include <dshow.h>
#include <qedit.h>
#include <strmif.h>

EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;

/**
 * @brief 已打开的设备实例
 *
 */
class BecamOpenedDevice {
private:
	// 当前使用的帧率
	uint32_t fps = 0;
	// 捕获筛选器（相机）
	IBaseFilter* captureFilter = nullptr;
	// 图像构建器（画布）
	IGraphBuilder* graphBuilder = nullptr;
	// 视频帧抓取器（处理）
	IBaseFilter* sampleGrabber = nullptr;
	// 视频帧抓取器（处理接口）
	ISampleGrabber* sampleGrabberIntf = nullptr;
	// 视频帧抓取器回调（处理接口回调）
	BecamSampleGrabberCallback* sampleGrabberCallback = nullptr;
	// 渲染器（空渲染器）
	IBaseFilter* nullRender;
	// 媒体控制器（开关控制）
	IMediaControl* mediaControl = nullptr;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param captureFilter 捕获筛选器（相机）
	 * @param frameInfo 视频帧信息
	 */
	BecamOpenedDevice(IBaseFilter* captureFilter, const VideoFrameInfo frameInfo);

	/**
	 * @brief 析构函数
	 */
	~BecamOpenedDevice();

	/**
	 * @brief 打开设备
	 *
	 * @param captureOuputPin 捕获筛选器输出端口
	 * @return 状态码
	 */
	StatusCode Open(IPin* captureOuputPin);

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
