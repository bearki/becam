#include "BecamDirectShow.hpp"
#include "BecamAmMediaType.hpp"
#include "BecamMonikerPropReader.hpp"
#include <pkg/StringConvert.hpp>

#include "BecamDeviceEnum.hpp"
#include <vector>

/**
 * @implements 实现构造函数
 */
BecamDirectShow::BecamDirectShow() {}

/**
 * @implements 实现析构函数
 */
BecamDirectShow::~BecamDirectShow() {
	// 释放已打开的设备
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}
}

/**
 * @implements 实现获取设备列表
 */
StatusCode BecamDirectShow::GetDeviceList(GetDeviceListReply& reply) {
	// 设备数量置空
	reply.deviceInfoListSize = 0;
	reply.deviceInfoList = nullptr;

	// 声明vector来储存设备列表
	std::vector<DeviceInfo> deviceVec;
	// 初始化设备枚举类
	auto deviceEnum = BecamDeviceEnum(true);
	// 开始枚举设备
	auto code = deviceEnum.EnumVideoDevices([&deviceVec](IMoniker* moniker) {
		// 获取设备友好名称
		std::string friendlyName = "";
		auto code = BecamDeviceEnum::GetMonikerFriendlyName(moniker, friendlyName);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备路径
		std::string devicePath = "";
		code = BecamDeviceEnum::GetMonikerDevicePath(moniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 构建设备信息
		DeviceInfo deviceInfo = {0};
		// 转换为C字符串
		if (friendlyName.size() > 0) {
			deviceInfo.name = new char[friendlyName.size() + 1];
			memcpy(deviceInfo.name, friendlyName.c_str(), friendlyName.size() + 1);
		}
		// 转换为C字符串
		if (devicePath.size() > 0) {
			deviceInfo.devicePath = new char[devicePath.size() + 1];
			memcpy(deviceInfo.devicePath, devicePath.c_str(), devicePath.size() + 1);
		}

		// 追加到结果中
		deviceVec.insert(deviceVec.end(), deviceInfo);

		// 继续枚举
		return true;
	});
	// 是否枚举失败
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "BecamDirectShow::GetDeviceList -> EnumVideoDevices failed, CODE: " << code << std::endl;
		return code;
	}
	// 是否有枚举到设备
	if (deviceVec.size() > 0) {
		// 拷贝设备列表
		reply.deviceInfoListSize = deviceVec.size();
		reply.deviceInfoList = new DeviceInfo[reply.deviceInfoListSize];
		memcpy(reply.deviceInfoList, deviceVec.data(), reply.deviceInfoListSize * sizeof(DeviceInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放设备列表
 */
void BecamDirectShow::FreeDeviceList(GetDeviceListReply& input) {
	// 检查
	if (input.deviceInfoListSize <= 0 || input.deviceInfoList == nullptr) {
		return;
	}

	// 遍历，执行释放操作
	for (size_t i = 0; i < input.deviceInfoListSize; i++) {
		// 获取引用
		auto item = input.deviceInfoList[i];
		// 释放友好名称
		if (item.name != nullptr) {
			delete[] item.name;
			item.name = nullptr;
		}
		// 释放设备路径
		if (item.devicePath != nullptr) {
			delete[] item.devicePath;
			item.devicePath = nullptr;
		}
	}

	// 释放整个列表
	delete[] input.deviceInfoList;
	input.deviceInfoListSize = 0;
	input.deviceInfoList = nullptr;
}

/**
 * @implements 实现获取设备配置列表
 */
StatusCode BecamDirectShow::GetDeviceConfigList(const std::string& devicePath, GetDeviceConfigListReply& reply) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 初始化设备枚举类
	auto deviceEnum = BecamDeviceEnum(true);
	// 声明设备实例
	IMoniker* moniker = nullptr;
	// 获取指定设备的引用
	auto code = deviceEnum.GetDeviceRef(devicePath, moniker);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}

	// 获取设备流能力
	VideoFrameInfo* list = nullptr;
	size_t listSize = 0;
	code = BecamDeviceEnum::GetDeviceStreamCaps(moniker, list, listSize);
	// 释放设备实例
	moniker->Release();
	moniker = nullptr;
	// 检查
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}

	// 赋值查询结果
	reply.videoFrameInfoListSize = listSize;
	reply.videoFrameInfoList = list;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放设备配置列表
 */
void BecamDirectShow::FreeDeviceConfigList(GetDeviceConfigListReply& input) {
	// 检查
	if (input.videoFrameInfoListSize <= 0 || input.videoFrameInfoList == nullptr) {
		return;
	}
	// 释放整个列表
	delete[] input.videoFrameInfoList;
	input.videoFrameInfoListSize = 0;
	input.videoFrameInfoList = nullptr;
}

/**
 * @implements 实现打开指定设备
 */
StatusCode BecamDirectShow::OpenDevice(const std::string& devicePath, const VideoFrameInfo& frameInfo) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}
	if (frameInfo.width <= 0 || frameInfo.height <= 0 || frameInfo.fps <= 0 || frameInfo.format <= 0) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 关闭已打开的设备
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}

	// 尝试打开设备
	this->openedDevice = new BecamOpenedDevice();
	auto code = this->openedDevice->Open(devicePath, frameInfo);
	// 是否打开成功
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 打开失败，关闭实例
		delete this->openedDevice;
		this->openedDevice = nullptr;
		std::cerr << "OpenDevice -> Open failed, CODE: " << code << std::endl;
	}

	// 返回结果
	return code;
}

/**
 * @implements 实现关闭设备
 */
void BecamDirectShow::CloseDevice() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已经打开了
	if (this->openedDevice == nullptr) {
		return;
	}

	// 释放已打开的设备
	delete this->openedDevice;
	this->openedDevice = nullptr;
}

/**
 * @implements 实现获取视频帧
 */
StatusCode BecamDirectShow::GetFrame(uint8_t*& data, size_t& size) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否打开
	if (this->openedDevice == nullptr) {
		return StatusCode::STATUS_CODE_DSHOW_ERR_DEVICE_NOT_OPEN;
	}

	// 获取视频帧
	return this->openedDevice->GetFrame(data, size);
}

/**
 * @implements 实现释放视频帧
 */
void BecamDirectShow::FreeFrame(uint8_t*& data) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (data == nullptr) {
		return;
	}

	// 检查设备是否打开
	if (this->openedDevice == nullptr) {
		return;
	}

	// 释放视频帧
	this->openedDevice->FreeFrame(data);
}
