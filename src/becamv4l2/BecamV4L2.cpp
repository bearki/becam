#include "BecamV4L2.hpp"
#include "Becamv4l2DeviceHelper.hpp"

/**
 * @implements 实现构造函数
 */
BecamV4L2::BecamV4L2() {}

/**
 * @implements 实现析构函数
 */
BecamV4L2::~BecamV4L2() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 释放已打开的设备
	// if (this->openedDevice != nullptr) {
	// 	delete this->openedDevice;
	// 	this->openedDevice = nullptr;
	// }
}

/**
 * @implements 实现获取设备列表
 */
StatusCode BecamV4L2::GetDeviceList(GetDeviceListReply* reply) {
	// 检查入参
	if (reply == nullptr) {
		return StatusCode::STATUS_CODE_V4L2_ERR_INPUT_PARAM;
	}
	// 执行设备列表获取
	return Becamv4l2DeviceHelper::GetDeviceList(reply->deviceInfoList, reply->deviceInfoListSize);
}

/**
 * @implements 实现释放设备列表
 */
void BecamV4L2::FreeDeviceList(GetDeviceListReply* input) {
	// 检查
	if (input == nullptr) {
		return;
	}
	// 执行释放
	Becamv4l2DeviceHelper::FreeDeviceList(input->deviceInfoList, input->deviceInfoListSize);
}

/**
 * @implements 实现获取设备配置列表
 */
StatusCode BecamV4L2::GetDeviceConfigList(const std::string devicePath, GetDeviceConfigListReply* reply) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty() || reply == nullptr) {
		return StatusCode::STATUS_CODE_V4L2_ERR_INPUT_PARAM;
	}

	// 初始化设备助手类
	Becamv4l2DeviceHelper deviceHelper;
	// 激活指定设备
	auto code = deviceHelper.ActivateDevice(devicePath);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}

	// 获取该设备支持的配置
	return deviceHelper.GetCurrentDeviceConfigList(reply->videoFrameInfoList, reply->videoFrameInfoListSize);
}

/**
 * @implements 实现释放设备配置列表
 */
void BecamV4L2::FreeDeviceConfigList(GetDeviceConfigListReply* input) {
	// 检查
	if (input == nullptr) {
		return;
	}
	// 执行释放
	Becamv4l2DeviceHelper::FreeDeviceConfigList(input->videoFrameInfoList, input->videoFrameInfoListSize);
}

