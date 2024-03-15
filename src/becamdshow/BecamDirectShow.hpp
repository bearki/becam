#ifndef _BECAMDSHOW_H_
#define _BECAMDSHOW_H_

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
	 * @param callback 回调函数（回调完成IMoniker将立即释放，回调返回false将立即停止枚举）
	 */
	StatusCode enumDevices(std::function<bool(IMoniker*)> callback);

	// 获取设备友好名称
	StatusCode getMonikerFriendlyName(IMoniker* pMoniker, std::string& friendlyName);

	// 获取设备路径
	StatusCode getMonikerDevicePath(IMoniker* pMoniker, std::string& devicePath);

	// 枚举针脚
	IPin* getPin(IBaseFilter* pFilter, PIN_DIRECTION dir);

	// 获取设备支持的流配置
	StatusCode getMonikerWithStreamConfig(IMoniker* pMoniker, const VideoFrameInfo* filter, VideoFrameInfo** reply,
										  size_t* replySize);

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
	 * @param reply 响应参数
	 * @return  状态码
	 */
	StatusCode GetDeviceList(GetDeviceListReply* reply);

	/**
	 * @brief 释放设备列表
	 *
	 * @param input 输入参数
	 */
	void FreeDeviceList(GetDeviceListReply* input);

	/**
	 * @brief 打开指定设备
	 *
	 * @param devicePath 设备路径
	 * @param frameInfo 设置的视频帧信息
	 * @return 状态码
	 */
	StatusCode OpenDevice(const std::string devicePath, const VideoFrameInfo* frameInfo);
};

#endif
