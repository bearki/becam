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
	// COM库是否初始化成功
	bool comInited = false;
	// 已打开设备实例
	BecamOpenedDevice* openedDevice = nullptr;

	/**
	 * @brief 枚举设备列表
	 *
	 * @param	callback	[in]	回调函数（回调结束将立即释放IMoniker，回调返回false将立即停止枚举）
	 * @return	状态码
	 */
	StatusCode enumDevices(std::function<bool(IMoniker*)> callback);

	/**
	 * @brief 获取设备友好名称
	 *
	 * @param	pMoniker	[in]	设备实例
	 * @param	devicePath	[out]	设备友好名称
	 * @return	状态码
	 */
	StatusCode getMonikerFriendlyName(IMoniker* pMoniker, std::string& friendlyName);

	/**
	 * @brief 获取设备路径
	 *
	 * @param	pMoniker	[in]	设备实例
	 * @param	devicePath	[out]	设备路径
	 * @return	状态码
	 */
	StatusCode getMonikerDevicePath(IMoniker* pMoniker, std::string& devicePath);

	/**
	 * @brief 获取设备
	 * @param	devicePath	[in]	设备路径
	 * @param	moniker		[out]	设备实例
	 * @return	状态码
	 */
	StatusCode getDevice(std::string devicePath, IMoniker*& moniker);

	// 枚举针脚
	static IPin* getPin(IBaseFilter* pFilter, PIN_DIRECTION dir);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param	streamConfig	[in]	设备流配置实例
	 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return	状态码
	 */
	StatusCode getDeviceStreamCaps(IAMStreamConfig* streamConfig, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param	captureOuputPin	[in]	捕获筛选器的输出端点
	 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return	状态码
	 */
	StatusCode getDeviceStreamCaps(IPin* captureOuputPin, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 枚举设备支持的流能力
	 *
	 * @param	captureFilter	[in]	捕获筛选器实例
	 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return	状态码
	 */
	StatusCode getDeviceStreamCaps(IBaseFilter* captureFilter, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param	pMoniker	[in]	设备实例
	 * @param	reply		[out]	设备流能力列表
	 * @param	replySize	[out]	设备流能力列表大小
	 * @return	状态码
	 */
	StatusCode getDeviceStreamCaps(IMoniker* pMoniker, VideoFrameInfo** reply, size_t* replySize);

	/**
	 * @brief 设置设备支持的流能力
	 *
	 * @param	captureOuputPin	[in]	捕获筛选器的输出端点
	 * @param	frameInfo		[in]	视频帧信息
	 * @return	状态码
	 */
	StatusCode setCaptureOuputPinStreamCaps(IPin* captureOuputPin, const VideoFrameInfo frameInfo);

public:
	/**
	 * @brief Construct a new Becam Direct Show object
	 */
	BecamDirectShow() {
		// 初始化COM库
		auto res = CoInitialize(nullptr);
		if (SUCCEEDED(res)) {
			this->comInited = true;
		} else {
			this->comInited = false;
		}
	};

	/**
	 * @brief Destroy the Becam Direct Show object
	 */
	~BecamDirectShow() {
		// 释放COM库
		if (this->comInited) {
			CoUninitialize();
			this->comInited = false;
		}
		// 释放已打开的设备
		if (this->openedDevice != nullptr) {
			delete this->openedDevice;
			this->openedDevice = nullptr;
		}
	};

	/**
	 * @brief 获取设备列表
	 *
	 * @param	reply	[out]	响应参数
	 * @return	状态码
	 */
	StatusCode GetDeviceList(GetDeviceListReply* reply);

	/**
	 * @brief 释放设备列表
	 *
	 * @param	input	[in]	输入参数
	 */
	void FreeDeviceList(GetDeviceListReply* input);

	/**
	 * @brief 打开指定设备
	 *
	 * @param	devicePath	[in]	设备路径
	 * @param	frameInfo	[in]	设置的视频帧信息
	 * @return	状态码
	 */
	StatusCode OpenDevice(const std::string devicePath, const VideoFrameInfo* frameInfo);

	/**
	 * @brief 关闭设备
	 */
	void CloseDevice();

	/**
	 * @brief 获取视频帧
	 *
	 * @param	data	视频帧流
	 * @param	size	视频帧流大小
	 * @return	状态码
	 */
	StatusCode GetFrame(uint8_t** data, size_t* size);

	/**
	 * @brief 释放视频帧
	 *
	 * @param	data	视频帧流
	 */
	void FreeFrame(uint8_t** data);
};

#endif
