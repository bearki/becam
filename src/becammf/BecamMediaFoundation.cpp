#include "BecamMediaFoundation.hpp"
#include <mfapi.h>
#include <mfidl.h>
#include <pkg/SafeRelease.hpp>
#include <pkg/StringConvert.hpp>

/**
 * @implements 实现构造函数
 */
BecamMediaFoundation::BecamMediaFoundation() {}

/**
 * @implements 实现析构函数
 */
BecamMediaFoundation::~BecamMediaFoundation() {
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
StatusCode BecamMediaFoundation::GetDeviceList(GetDeviceListReply& reply) {
	// 初始化设备助手对象
	BecammfDeviceHelper deviceHelper;
	// 执行设备列表获取
	return deviceHelper.GetDeviceList(reply.deviceInfoList, reply.deviceInfoListSize);
}

/**
 * @implements 实现释放设备列表
 */
void BecamMediaFoundation::FreeDeviceList(GetDeviceListReply& input) {
	// 执行释放
	BecammfDeviceHelper::FreeDeviceList(input.deviceInfoList, input.deviceInfoListSize);
}

/**
 * @implements 实现获取设备配置列表
 */
StatusCode BecamMediaFoundation::GetDeviceConfigList(const std::string& devicePath, GetDeviceConfigListReply& reply) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 初始化设备助手类
	BecammfDeviceHelper deviceHelper;
	// 激活指定设备
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
void BecamMediaFoundation::FreeDeviceConfigList(GetDeviceConfigListReply& input) {
	// 执行释放
	BecammfDeviceHelper::FreeDeviceConfigList(input.videoFrameInfoList, input.videoFrameInfoListSize);
}

/**
 * @implements 实现打开指定设备
 */
StatusCode BecamMediaFoundation::OpenDevice(const std::string& devicePath, const VideoFrameInfo& frameInfo) {
	// 检查参数
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 关闭已打开的设备
	this->openedDevice->CloseDevice();

	// 激活设备
	auto code = this->openedDevice->ActivateDevice(devicePath);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}
	// 激活设备源读取器
	return this->openedDevice->ActivateCurrentDeviceStreaming(frameInfo);
}

/**
 * @implements 实现关闭设备
 */
void BecamMediaFoundation::CloseDevice() {
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
StatusCode BecamMediaFoundation::GetFrame(uint8_t*& data, size_t& size) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已打开
	return this->openedDevice->GetFrame(data, size);
}

/**
 * @implements 实现释放视频帧
 */
void BecamMediaFoundation::FreeFrame(uint8_t*& data) {
	// 释放视频帧
	BecammfDeviceHelper::FreeFrame(data);
}
