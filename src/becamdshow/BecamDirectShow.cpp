#include "BecamDirectShow.hpp"
#include "BecamAmMediaType.hpp"
#include "BecamMonikerPropReader.hpp"
#include "BecamStringConvert.hpp"

#include "BecamEnumDevice.hpp"
#include <vector>

/**
 * @brief 构造函数
 */
BecamDirectShow::BecamDirectShow() {}

/**
 * @brief 析构函数
 */
BecamDirectShow::~BecamDirectShow() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 释放已打开的设备
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}
}

/**
 * @brief 获取设备列表
 *
 * @param reply [out] 响应参数
 * @return 状态码
 */
StatusCode BecamDirectShow::GetDeviceList(GetDeviceListReply* reply) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (reply == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 设备数量置空
	reply->deviceInfoListSize = 0;
	reply->deviceInfoList = nullptr;

	// 声明vector来储存设备列表
	std::vector<DeviceInfo> deviceVec;
	// 初始化设备枚举类
	auto enumDevice = BecamEnumDevice(true);
	// 开始枚举设备
	auto code = enumDevice.EnumVideoDevices([this, &deviceVec](IMoniker* moniker) {
		// 获取设备友好名称
		std::string friendlyName = "";
		auto code = BecamEnumDevice::GetMonikerFriendlyName(moniker, friendlyName);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备路径
		std::string devicePath = "";
		code = BecamEnumDevice::GetMonikerDevicePath(moniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备位置信息
		std::string locationInfo = "";

		// 获取设备流能力
		VideoFrameInfo* list = nullptr;
		size_t listSize = 0;
		code = BecamEnumDevice::GetDeviceStreamCaps(moniker, &list, &listSize);
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
		// 转换为C字符串
		if (locationInfo.size() > 0) {
			deviceInfo.locationInfo = nullptr;
			memcpy(deviceInfo.locationInfo, locationInfo.c_str(), locationInfo.size() + 1);
		}
		// 赋值视频帧信息
		deviceInfo.frameInfoListSize = listSize;
		deviceInfo.frameInfoList = list;

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
		// 转换为定长数组
		reply->deviceInfoListSize = deviceVec.size();
		reply->deviceInfoList = new DeviceInfo[deviceVec.size()];
		memcpy(reply->deviceInfoList, deviceVec.data(), deviceVec.size() * sizeof(DeviceInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 释放设备列表
 *
 * @param input [in] 输入参数
 */
void BecamDirectShow::FreeDeviceList(GetDeviceListReply* input) {
	// 检查
	if (input == nullptr) {
		return;
	}
	if (input->deviceInfoListSize <= 0 || input->deviceInfoList == nullptr) {
		return;
	}

	// 遍历，执行释放操作
	for (size_t i = 0; i < input->deviceInfoListSize; i++) {
		// 获取引用
		auto item = input->deviceInfoList[i];
		// 释放友好名称
		if (item.name != nullptr) {
			delete item.name;
			item.name = nullptr;
		}
		// 释放设备路径
		if (item.devicePath != nullptr) {
			delete item.devicePath;
			item.devicePath = nullptr;
		}
		// 释放位置信息
		if (item.locationInfo != nullptr) {
			delete item.locationInfo;
			item.locationInfo = nullptr;
		}
		// 释放视频帧列表
		if (item.frameInfoListSize > 0 && item.frameInfoList != nullptr) {
			delete item.frameInfoList;
			item.frameInfoListSize = 0;
			item.frameInfoList = nullptr;
		}
	}

	// 释放整个列表
	delete input->deviceInfoList;
	input->deviceInfoListSize = 0;
	input->deviceInfoList = nullptr;
}

/**
 * @brief 打开指定设备
 *
 * @param devicePath [in] 设备路径
 * @param frameInfo [in] 设置的视频帧信息
 * @return 状态码
 */
StatusCode BecamDirectShow::OpenDevice(const std::string devicePath, const VideoFrameInfo* frameInfo) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (devicePath.empty() || frameInfo == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}
	if (frameInfo->width <= 0 || frameInfo->height <= 0 || frameInfo->fps <= 0 || frameInfo->format <= 0) {
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
 * @brief 关闭设备
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
 * @brief 获取视频帧
 *
 * @param data 视频帧流
 * @param size 视频帧流大小
 * @return 状态码
 */
StatusCode BecamDirectShow::GetFrame(uint8_t** data, size_t* size) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (data == nullptr || size == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 检查设备是否打开
	if (this->openedDevice == nullptr) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_OPEN;
	}

	// 获取视频帧
	return this->openedDevice->GetFrame(data, size);
}

/**
 * @brief 释放视频帧
 *
 * @param data 视频帧流
 */
void BecamDirectShow::FreeFrame(uint8_t** data) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查入参
	if (data == nullptr || *data == nullptr) {
		return;
	}

	// 检查设备是否打开
	if (this->openedDevice == nullptr) {
		return;
	}

	// 释放视频帧
	this->openedDevice->FreeFrame(data);
}
