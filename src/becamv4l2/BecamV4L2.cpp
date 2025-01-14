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
		return StatusCode::STATUS_CODE_MF_ERR_INPUT_PARAM;
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
