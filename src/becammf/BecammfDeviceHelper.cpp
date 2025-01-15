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
	this->CloseCurrentDevice();
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
 * @implements 关闭当前设备
 */
void BecammfDeviceHelper::CloseCurrentDevice() {
	// 已打开的设备源读取器需要释放
	this->StopCurrentDeviceStreaming();
	// 已打开的设备需要关闭设备
	if (this->activatedDevice != nullptr) {
		// 关闭设备
		this->activatedDevice->Shutdown();
	}
	// 安全的释放设备
	SafeRelease(&this->activatedDevice);
}

/**
 * @implements 停止当前设备取流
 */
void BecammfDeviceHelper::StopCurrentDeviceStreaming() {
	// 安全释放设备源读取器
	SafeRelease(&this->activatedReader);
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
		auto friendlyName = WStringToString(friendlyNameW);
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
		auto symbolicLink = WStringToString(symbolicLinkW);
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
StatusCode BecammfDeviceHelper::ActivateDevice(const std::string& devicePath) {
	// 参数检查
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 释放已激活的设备
	this->CloseCurrentDevice();

	// 托管属性存储器，使其自动释放
	auto attributesHelper = BecammfAttributesHelper();
	// 创建属性存储器
	HRESULT res = MFCreateAttributes(attributesHelper.AttributesAddress(), 1);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDevice -> MFCreateAttributes() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_ATTR_STORE;
	}

	// 赋值设备属性类型
	res = attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(res)) {
		std::cerr
			<< "BecammfDeviceHelper::ActivateDevice -> attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE) failed, "
			   "HRESULT: "
			<< res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}
	// 赋值设备属性符号链接
	auto devicePathW = StringToWString(devicePath);
	res = attributesHelper.Attributes()->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, (LPCWSTR)devicePathW.c_str());
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDevice -> "
					 "attributesHelper.Attributes()->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK) "
					 "failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}

	// 激活设备
	res = MFCreateDeviceSource(attributesHelper.Attributes(), &this->activatedDevice);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDevice -> MFCreateDeviceSource() failed, HRESULT: " << res << std::endl;
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
 * @implements 实现释放已获取的设备支持的配置列表
 */
void BecammfDeviceHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 执行释放
	BecammfDeviceConfigHelper::FreeDeviceConfigList(input, inputSize);
}

/**
 * @implements 实现激活当前设备取流
 */
StatusCode BecammfDeviceHelper::ActivateCurrentDeviceStreaming(const VideoFrameInfo frameInfo) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已激活
	if (this->activatedDevice == nullptr) {
		return StatusCode::STATUS_CODE_MF_ERR_DEVICE_UNACTIVATE;
	}

	// 释放已存在的设备源读取器
	this->StopCurrentDeviceStreaming();

	// 托管属性存储器，使其自动释放
	auto attributesHelper = BecammfAttributesHelper();
	// 创建属性对象
	auto res = MFCreateAttributes(attributesHelper.AttributesAddress(), 1);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> MFCreateAttributes() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_ATTR_STORE;
	}

// 条件设置低延迟模式
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	// 设置低延迟模式
	res = attributesHelper.Attributes()->SetUINT32(MF_LOW_LATENCY, true);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> attributesHelper.Attributes()->SetUINT32(MF_LOW_LATENCY, true) failed, "
					 "HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_ATTR_STORE;
	}
#endif

	// 激活源读取器
	res = MFCreateSourceReaderFromMediaSource(this->activatedDevice, attributesHelper.Attributes(), &this->activatedReader);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> MFCreateSourceReaderFromMediaSource() failed, HRESULT: " << res
				  << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_SOURCE_READER;
	}

	// 媒体资源类型索引下标
	int typeIndex = 0;
	// 查询到的媒体资源类型
	IMFMediaType* pType = nullptr;
	// TODO：不想遍历流索引，先固定为MF_SOURCE_READER_FIRST_VIDEO_STREAM吧
	// 枚举设备支持的流格式
	while (SUCCEEDED(this->activatedReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, typeIndex, &pType))) {
		// 索引叠加
		typeIndex++;

		// 获取帧格式
		GUID subtype;
		res = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
		if (FAILED(res)) {
			// 释放媒体资源类型
			SafeRelease(&pType);
			std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> pType->GetGUID(MF_MT_SUBTYPE, &subtype) "
						 "failed, HRESULT: "
					  << res << std::endl;
			// 忽略，继续下一个
			continue;
		}
		// 检查帧格式是否一致
		if (BecammfDeviceConfigHelper::GuidToFourcc(subtype) != frameInfo.format) {
			// 不一致，释放查询到的
			SafeRelease(&pType);
			// 忽略，继续下一个
			continue;
		}

		// 获取帧大小
		UINT32 width = 0;
		UINT32 height = 0;
		res = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
		if (FAILED(res)) {
			// 释放媒体资源类型
			SafeRelease(&pType);
			std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> MFGetAttributeRatio(pType, MF_MT_FRAME_SIZE) "
						 "failed, HRESULT: "
					  << res << std::endl;
			// 忽略，继续下一个
			continue;
		}
		// 检查大小是否一致
		if (width != frameInfo.width || height != frameInfo.height) {
			// 不一致，释放查询到的
			SafeRelease(&pType);
			// 忽略，继续下一个
			continue;
		}

		// 获取帧率
		UINT32 numerator = 0;
		UINT32 denominator = 0;
		res = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);
		if (FAILED(res)) {
			// 释放媒体资源类型
			SafeRelease(&pType);
			std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> MFGetAttributeRatio(pType, MF_MT_FRAME_RATE) failed, HRESULT: "
					  << res << std::endl;
			// 忽略，继续下一个
			continue;
		}
		// 检查帧率是否一致
		if (numerator / denominator != frameInfo.fps) {
			// 不一致，释放查询到的
			SafeRelease(&pType);
			// 忽略，继续下一个
			continue;
		}

		// 查询成功，跳出查询，不释放媒体资源类型，保持对其的引用
		break;
	}

	// 检查是否查询到对应的配置
	if (pType == nullptr) {
		return StatusCode::STATUS_CODE_MF_ERR_MEDIA_TYPE_NOT_FOUND;
	}

	// 设置输出格式
	res = this->activatedReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pType);
	if (FAILED(res)) {
		// 释放引用的媒体资源类型
		SafeRelease(&pType);
		std::cerr << "BecammfDeviceHelper::ActivateDeviceReader -> SetCurrentMediaType() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_SET_MEDIA_TYPE;
	}
	// 释放引用的媒体资源类型
	SafeRelease(&pType);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现关闭设备
 */
void BecammfDeviceHelper::CloseDevice() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 关闭当前设备
	this->CloseCurrentDevice();
}

/**
 * @implements 实现获取视频帧
 */
StatusCode BecammfDeviceHelper::GetFrame(uint8_t*& reply, size_t& replySize) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已激活
	if (this->activatedDevice == nullptr || this->activatedReader == nullptr) {
		return StatusCode::STATUS_CODE_MF_ERR_DEVICE_UNACTIVATE;
	}

	// 声明一些要用的东西
	DWORD streamIndex, sampleFlags;
	LONGLONG timestamp;
	IMFSample* pSample = nullptr;
	// 读取一帧
	auto res = this->activatedReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &sampleFlags, &timestamp, &pSample);
	if (FAILED(res)) {
		std::cerr
			<< "BecammfDeviceHelper::GetFrame -> this->activatedReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, HRESULT: "
			<< res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_GET_FRAME;
	}

	// 是否读取到帧
	if (pSample == nullptr) {
		return StatusCode::STATUS_CODE_MF_ERR_GET_FRAME_EMPTY;
	}

	// 获取媒体缓冲区
	IMFMediaBuffer* pBuffer = nullptr;
	res = pSample->ConvertToContiguousBuffer(&pBuffer);
	if (FAILED(res)) {
		// 安全释放帧
		SafeRelease(&pSample);
		std::cerr << "BecammfDeviceHelper::GetFrame -> pSample->ConvertToContiguousBuffer() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CONVERT_FRAME_BUFFER;
	}

	// 锁定缓冲区并获取数据指针
	BYTE* pData = nullptr;
	DWORD bufferLength = 0;
	res = pBuffer->Lock(&pData, nullptr, &bufferLength);
	if (FAILED(res)) {
		// 安全释放缓冲区
		SafeRelease(&pBuffer);
		// 安全释放帧
		SafeRelease(&pSample);
		std::cerr << "BecammfDeviceHelper::GetFrame -> pBuffer->Lock() failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_LOCK_FRAME_BUFFER;
	}

	// 是否需要拷贝缓冲区
	if (bufferLength > 0) {
		// 拷贝缓冲区
		replySize = bufferLength;
		reply = new uint8_t[replySize];
		memcpy(reply, pData, replySize);
	}

	// 解锁缓冲区
	pBuffer->Unlock();
	// 安全释放缓冲区
	SafeRelease(&pBuffer);
	// 安全释放帧
	SafeRelease(&pSample);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放已获取的视频帧
 */
void BecammfDeviceHelper::FreeFrame(uint8_t*& reply) {
	if (reply == nullptr) {
		return;
	}

	delete[] reply;
	reply = nullptr;
}
