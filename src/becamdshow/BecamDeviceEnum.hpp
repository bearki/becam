#pragma once

#ifndef _BECAM_ENUM_DEVICE_HPP_
#define _BECAM_ENUM_DEVICE_HPP_

#include <becam/becam.h>
#include <dshow.h>
#include <functional>
#include <string>

/**
 * @brief 枚举设备
 */
class BecamDeviceEnum {
private:
	// COM库是否已初始化
	bool comInited = false;
	// 设备枚举器实例
	ICreateDevEnum* devEnumInstance = nullptr;
	// 视频输入设备枚举器实例
	IEnumMoniker* videoInputEnumInstance = nullptr;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param initCom [in] 是否初始化COM库
	 */
	BecamDeviceEnum(bool initCom = true);

	/**
	 * @brief 析构函数
	 */
	~BecamDeviceEnum();

	/**
	 * @brief 枚举所有视频设备
	 *
	 * @param callback [in] 回调函数（回调结束将立即释放IMoniker，回调返回false将立即停止枚举）
	 * @return 状态码
	 */
	StatusCode EnumVideoDevices(std::function<bool(IMoniker*)> callback);

	/**
	 * @brief 获取设备友好名称
	 *
	 * @param moniker [in] 设备实例
	 * @param friendlyName [out] 设备友好名称
	 * @return 状态码
	 */
	static StatusCode GetMonikerFriendlyName(IMoniker* moniker, std::string& friendlyName);

	/**
	 * @brief 获取设备路径
	 *
	 * @param moniker [in] 设备实例
	 * @param devicePath [out] 设备路径
	 * @return 状态码
	 */
	static StatusCode GetMonikerDevicePath(IMoniker* moniker, std::string& devicePath);

	/**
	 * @brief 获取设备引用
	 * @param devicePath [in] 设备路径
	 * @param moniker [out] 设备实例
	 * @return 状态码
	 */
	StatusCode GetDeviceRef(const std::string devicePath, IMoniker*& moniker);

	/**
	 * @brief 获取捕获筛选器的输出端口
	 *
	 * @param captureFilter [in] 捕获筛选器实例
	 * @param dir [in] 输出方向
	 * @return 捕获筛选器的输出端口
	 */
	static IPin* GetPin(IBaseFilter* captureFilter, PIN_DIRECTION dir);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param streamConfig [in] 设备流配置实例
	 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return 状态码
	 */
	static StatusCode GetDeviceStreamCaps(IAMStreamConfig* streamConfig, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param captureOuputPin [in] 捕获筛选器的输出端点
	 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return 状态码
	 */
	static StatusCode GetDeviceStreamCaps(IPin* captureOuputPin, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 枚举设备支持的流能力
	 *
	 * @param captureFilter [in] 捕获筛选器实例
	 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
	 * @return 状态码
	 */
	static StatusCode GetDeviceStreamCaps(IBaseFilter* captureFilter, std::function<bool(AM_MEDIA_TYPE*)> callback);

	/**
	 * @brief 获取设备支持的流能力
	 *
	 * @param moniker [in]设备实例
	 * @param reply [out] 视频帧信息列表引用
	 * @param replySize [out] 视频帧信息列表大小引用
	 * @return 状态码
	 */
	static StatusCode GetDeviceStreamCaps(IMoniker* moniker, VideoFrameInfo*& reply, size_t& replySize);

	/**
	 * @brief 设置设备支持的流能力
	 *
	 * @param captureOuputPin [in] 捕获筛选器的输出端点
	 * @param frameInfo [in] 视频帧信息
	 * @return 状态码
	 */
	static StatusCode SetCaptureOuputPinStreamCaps(IPin* captureOuputPin, const VideoFrameInfo frameInfo);
};

#endif
