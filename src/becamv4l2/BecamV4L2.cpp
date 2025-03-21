#include "BecamV4L2.hpp"

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
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}
}

/**
 * @implements 实现获取设备列表
 */
StatusCode BecamV4L2::GetDeviceList(GetDeviceListReply& reply) {
	// 执行设备列表获取
	return Becamv4l2DeviceHelper::GetDeviceList(reply.deviceInfoList, reply.deviceInfoListSize);
}

/**
 * @implements 实现释放设备列表
 */
void BecamV4L2::FreeDeviceList(GetDeviceListReply& input) {
	// 执行释放
	Becamv4l2DeviceHelper::FreeDeviceList(input.deviceInfoList, input.deviceInfoListSize);
}

/**
 * @implements 实现获取设备配置列表
 */
StatusCode BecamV4L2::GetDeviceConfigList(const std::string& devicePath, GetDeviceConfigListReply& reply) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 初始化设备助手类
	Becamv4l2DeviceHelper deviceHelper;
	// 激活指定设备（激活的设备会随着设备助手类作用域自动关闭）
	auto code = deviceHelper.ActivateDevice(devicePath);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}

	// 获取该设备支持的配置
	return deviceHelper.GetCurrentDeviceConfigList(reply.videoFrameInfoList, reply.videoFrameInfoListSize);
}

/**
 * @implements 实现释放设备配置列表
 */
void BecamV4L2::FreeDeviceConfigList(GetDeviceConfigListReply& input) {
	// 执行释放
	Becamv4l2DeviceHelper::FreeDeviceConfigList(input.videoFrameInfoList, input.videoFrameInfoListSize);
}

/**
 * @implements 实现打开指定设备
 */
StatusCode BecamV4L2::OpenDevice(const std::string& devicePath, const VideoFrameCaptureInfo& capInfo) {
	// 检查参数
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 关闭已打开的设备
	this->openedDevice->CloseDevice();

	// 激活设备
	auto code = this->openedDevice->ActivateDevice(devicePath, O_RDWR);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}
	// 激活设备源读取器
	return this->openedDevice->ActivateDeviceStreaming(capInfo);
}

/**
 * @implements 实现关闭设备
 */
void BecamV4L2::CloseDevice() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 关闭已打开的设备
	if (this->openedDevice != nullptr) {
		this->openedDevice->CloseDevice();
	}
}

/**
 * @implements 实现获取视频帧
 */
StatusCode BecamV4L2::GetFrame(uint8_t*& data, size_t& size) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已打开
	return this->openedDevice->GetFrame(data, size);
}

/**
 * @implements 实现释放视频帧
 */
void BecamV4L2::FreeFrame(uint8_t*& data) {
	// 释放获取到的帧
	Becamv4l2DeviceHelper::FreeFrame(data);
}
