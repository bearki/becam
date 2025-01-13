#include "BecammfDeviceHelper.hpp"
#include "BecammfAttributesHelper.hpp"
#include "BecammfDeviceConfigHelper.hpp"
#include <iostream>
#include <mfapi.h>
#include <pkg/StringConvert.hpp>
#include <vector>

/**
 * @implements 实现构造函数
 */
BecammfDeviceHelper::BecammfDeviceHelper() {
	// 初始化Com库
	this->_ComInitResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(this->_ComInitResult)) {
		// 初始化Media Foundation库
		this->_MFStartupResult = MFStartup(MF_VERSION);
	}
}

/**
 * @implements 实现析构函数
 */
BecammfDeviceHelper::~BecammfDeviceHelper() {
	// 释放当前设备
	this->ReleaseCurrent();
	// COM库初始化成功时需要释放
	if (SUCCEEDED(this->_ComInitResult)) {
		// 释放Com库
		CoUninitialize();
	}
	// Media Foundation库初始化成功时需要关闭
	if (SUCCEEDED(this->_MFStartupResult)) {
		// 关闭Media Foundation库
		MFShutdown();
	}
}

/**
 * @implements 实现释放当前设备
 */
void BecammfDeviceHelper::ReleaseCurrent() {
	// 已打开的设备需要关闭设备
	if (this->activatedDevice != nullptr) {
		// 关闭设备
		activatedDevice->Shutdown();
	}
	// 释放设备
	SafeRelease(&activatedDevice);
}

/**
 * @implements 实现获取设备列表
 */
StatusCode BecammfDeviceHelper::GetDeviceList(DeviceInfo*& reply, size_t& replySize) {
	// 设备数量置空
	replySize = 0;
	reply = nullptr;

	// 托管属性存储器，使其自动释放
	auto attributesHelper = BecammfAttributesHelper();
	// 创建属性存储器
	HRESULT res = MFCreateAttributes(attributesHelper.AttributesAddress(), 1);
	if (FAILED(res)) {
		std::cerr << "BecamMediaFoundation::GetDeviceList -> MFCreateAttributes(attributesHelper.AttributesAddress(), 1) failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_ATTR_STORE;
	}

	// 赋值设备属性类型
	res = attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(res)) {
		std::cerr << "BecamMediaFoundation::GetDeviceList -> attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE) "
					 "failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}

	// 开始枚举设备
	IMFActivate** ppDevices = nullptr;
	UINT32 count = 0;
	res = MFEnumDeviceSources(attributesHelper.Attributes(), &ppDevices, &count);
	if (FAILED(res)) {
		std::cerr << "BecamMediaFoundation::GetDeviceList -> MFEnumDeviceSources() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_DEVICE_ENUM;
	}
	if (count == 0) {
		return StatusCode::STATUS_CODE_SUCCESS;
	}

	// 构建临时设备列表
	std::vector<DeviceInfo> deviceList;
	// 遍历设备列表
	for (size_t i = 0; i < count; i++) {
		// 提取设备
		auto device = ppDevices[i];

		// 提取设备友好名称
		WCHAR* friendlyNameW = nullptr;
		UINT32 friendlyNameLength = 0;
		device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendlyNameW, &friendlyNameLength);
		auto friendlyName = WcharToChar(friendlyNameW, CP_UTF8);
		if (friendlyNameW != nullptr) {
			CoTaskMemFree(friendlyNameW);
		}

		// 提取设备唯一路径
		WCHAR* symbolicLinkW = nullptr;
		UINT32 symbolicLinkLength = 0;
		res = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symbolicLinkW, &symbolicLinkLength);
		if (FAILED(res)) {
			// 释放设备
			SafeRelease(&device);
			// 忽略该设备，继续下一个设备
			continue;
		}
		auto symbolicLink = WcharToChar(symbolicLinkW, CP_UTF8);
		if (symbolicLinkW != nullptr) {
			CoTaskMemFree(symbolicLinkW);
		}

		// 构建设备信息
		DeviceInfo deviceInfo = {0};
		// 转换为C字符串
		if (friendlyName.size() > 0) {
			deviceInfo.name = new char[friendlyName.size() + 1];
			memcpy(deviceInfo.name, friendlyName.c_str(), friendlyName.size() + 1);
		}
		// 转换为C字符串
		if (symbolicLink.size() > 0) {
			deviceInfo.devicePath = new char[symbolicLink.size() + 1];
			memcpy(deviceInfo.devicePath, symbolicLink.c_str(), symbolicLink.size() + 1);
		}

		// 添加设备信息到临时列表
		deviceList.push_back(deviceInfo);
		// 释放设备
		SafeRelease(&device);
	}
	// 释放设备列表
	CoTaskMemFree(ppDevices);
	ppDevices = nullptr;

	// 拷贝零时列表到响应结果
	if (deviceList.size() > 0) {
		replySize = deviceList.size();
		reply = new DeviceInfo[replySize];
		memcpy(reply, deviceList.data(), replySize * sizeof(DeviceInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放设备列表
 */
void BecammfDeviceHelper::FreeDeviceList(DeviceInfo*& input, size_t& inputSize) {
	// 检查
	if (inputSize <= 0 || input == nullptr) {
		return;
	}

	// 遍历，执行释放操作
	for (size_t i = 0; i < inputSize; i++) {
		// 获取引用
		auto item = input[i];
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
	delete[] input;
	inputSize = 0;
	input = nullptr;
}

/**
 * @implements 实现激活指定设备
 */
StatusCode BecammfDeviceHelper::ActivateDevice(std::string devicePath) {
	// 参数检查
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_MF_ERR_INPUT_PARAM;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 释放已激活的设备
	this->ReleaseCurrent();

	// 托管属性存储器，使其自动释放
	auto attributesHelper = BecammfAttributesHelper();
	// 创建属性存储器
	HRESULT res = MFCreateAttributes(attributesHelper.AttributesAddress(), 1);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::Activate -> MFCreateAttributes() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_ATTR_STORE;
	}

	// 赋值设备属性类型
	res = attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::Activate -> attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE) failed, "
					 "HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}
	// 赋值设备属性符号链接
	auto devicePathW = StringToWString(devicePath);
	res = attributesHelper.Attributes()->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, (LPCWSTR)devicePathW.c_str());
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::Activate -> "
					 "attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK) "
					 "failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}

	// 激活设备
	res = MFCreateDeviceSource(attributesHelper.Attributes(), &this->activatedDevice);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::Activate -> MFCreateDeviceSource() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_DEVICE_SOURCE;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取当前设备支持的配置列表
 */
StatusCode BecammfDeviceHelper::GetCurrentDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已激活
	if (this->activatedDevice == nullptr) {
		return StatusCode::STATUS_CODE_MF_ERR_DEVICE_UNACTIVATE;
	}

	// 初始化设备配置助手类
	auto configHelper = BecammfDeviceConfigHelper(this->activatedDevice);
	// 查询设备支持的配置列表
	return configHelper.GetDeviceConfigList(reply, replySize);
}

/**
 * @brief 释放已获取的设备支持的配置列表
 *
 * @param input [in && out] 已获取的视频帧信息列表引用
 * @param inputSize [in && out] 已获取的视频帧信息列表大小引用
 */
void BecammfDeviceHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 执行释放
	BecammfDeviceConfigHelper::FreeDeviceConfigList(input, inputSize);
}
