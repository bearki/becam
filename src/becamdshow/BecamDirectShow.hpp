#ifndef _BECAMDSHOW_HPP_
#define _BECAMDSHOW_HPP_

#include <becam/becam.h>
#include <dshow.h>
#include <iostream>

/**
 * @brief 基于DirectShow实现Becam接口
 *
 */
class BecamDirectShow {
private:
	// COM库是否初始化成功
	bool comInited = false;

	// 获取设备友好名称
	StatusCode getMonikerFriendlyName(IMoniker* pMoniker, std::string& friendlyName);

	// 获取设备路径
	StatusCode getMonikerDevicePath(IMoniker* pMoniker, std::string& devicePath);

	// 枚举针脚
	IPin* getPin(IBaseFilter* pFilter, PIN_DIRECTION dir);

	// 获取设备支持的流配置
	StatusCode getMonikerWithStreamConfig(IMoniker* pMoniker, VideoFrameInfo** reply, size_t* replySize);

public:
	/**
	 * @brief Construct a new Becam Direct Show object
	 */
	BecamDirectShow();

	/**
	 * @brief Destroy the Becam Direct Show object
	 */
	~BecamDirectShow();

	/**
	 * @brief 获取设备列表
	 *
	 * @param   reply   响应参数
	 * @return  状态码
	 */
	StatusCode GetDeviceList(GetDeviceListReply* reply);

	/**
	 * @brief 释放设备列表
	 *
	 * @param input 输入参数
	 */
	void FreeDeviceList(GetDeviceListReply* input);
};

#endif
