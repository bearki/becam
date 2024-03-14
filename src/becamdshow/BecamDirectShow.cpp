#include "BecamDirectShow.hpp"
#include "MonikerPropReader.hpp"
#include "StringConvert.hpp"

#include <vector>

/**
 * @brief 构造函数
 */
BecamDirectShow::BecamDirectShow() {
	// 初始化COM库
	auto res = CoInitialize(nullptr);
	if (SUCCEEDED(res)) {
		this->comInited = true;
	} else {
		this->comInited = false;
	}
}

/**
 * @brief 析构函数
 */
BecamDirectShow::~BecamDirectShow() {
	// 是否需要释放COM库
	if (this->comInited) {
		// 释放COM库
		CoUninitialize();
	}
}

// 获取设备友好名称
StatusCode BecamDirectShow::getMonikerFriendlyName(IMoniker* pMoniker, std::string& friendlyName) {
	// 检查参数
	if (pMoniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = MonikerPropReader(L"FriendlyName");
	auto res = reader.read(pMoniker);
	if (res.first != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败
		return res.first;
	}

	// UTF16转换为UTF8
	// 一般可以加一下第二个参数，顺便切换编码
	friendlyName = WcharToChar(res.second.bstrVal, CP_UTF8);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

// 获取设备路径
StatusCode BecamDirectShow::getMonikerDevicePath(IMoniker* pMoniker, std::string& devicePath) {
	// 检查参数
	if (pMoniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = MonikerPropReader(L"DevicePath");
	auto res = reader.read(pMoniker);
	if (res.first != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败
		return res.first;
	}

	// UTF16转换为UTF8
	// 一般可以加一下第二个参数，顺便切换编码
	devicePath = WcharToChar(res.second.bstrVal, CP_UTF8);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

// 枚举针脚
IPin* BecamDirectShow::getPin(IBaseFilter* pFilter, PIN_DIRECTION dir) {
	IEnumPins* enumPins;
	if (FAILED(pFilter->EnumPins(&enumPins)))
		return nullptr;

	IPin* pin;
	while (enumPins->Next(1, &pin, nullptr) == S_OK) {
		PIN_DIRECTION d;
		pin->QueryDirection(&d);
		if (d == dir) {
			enumPins->Release();
			return pin;
		}
		pin->Release();
	}
	enumPins->Release();
	return nullptr;
}

// 获取设备支持的流配置
StatusCode BecamDirectShow::getMonikerWithStreamConfig(IMoniker* pMoniker, VideoFrameInfo** reply, size_t* replySize) {
	// 检查参数
	if (pMoniker == nullptr || reply == nullptr || replySize == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 绑定到IBaseFilter接口
	IBaseFilter* pFilter = nullptr;
	auto res = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
	if (FAILED(res)) {
		// 绑定接口失败了
		return StatusCode::STATUS_CODE_ERR_GET_VIDEO_FRAME;
	}

	// 获取PIN接口
	auto pin = this->getPin(pFilter, PINDIR_OUTPUT);
	if (pin == nullptr) {
		// 释放IBaseFilter接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取PIN接口失败
		return StatusCode::STATUS_CODE_ERR_GET_VIDEO_FRAME;
	}

	// 获取流配置接口
	IAMStreamConfig* pStreamConfig = nullptr;
	res = pin->QueryInterface(IID_IAMStreamConfig, (void**)&pStreamConfig);
	if (FAILED(res)) {
		// 释放PIN接口
		pin->Release();
		pin = nullptr;
		// 释放IBaseFilter接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取流配置接口失败
		return StatusCode::STATUS_CODE_ERR_GET_VIDEO_FRAME;
	}

	// 声明用来接收支持的流配置总数量
	int count, size;
	// 获取配置总数量
	res = pStreamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(res)) {
		// 释放流配置接口
		pStreamConfig->Release();
		pStreamConfig = nullptr;
		// 释放PIN接口
		pin->Release();
		pin = nullptr;
		// 释放IBaseFilter接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取配置总数量失败
		return StatusCode::STATUS_CODE_ERR_GET_VIDEO_FRAME;
	}

	// 初始构建想要的列表
	auto tmpList = new VideoFrameInfo[count];
	// 列表实际有效长度
	size_t validSize = 0;
	// 遍历所有流配置
	for (int i = 0; i < count; ++i) {
		AM_MEDIA_TYPE* pmt = NULL;
		VIDEO_STREAM_CONFIG_CAPS scc;
		res = pStreamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(res)) {
			// 分析失败，继续下一个
			continue;
		}
		if (pmt->majortype != MEDIATYPE_Video || pmt->formattype != FORMAT_VideoInfo || pmt->pbFormat == nullptr) {
			// 信息不是想要的
			continue;
		}

		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		// 赋值视频帧信息
		tmpList[validSize].width = videoInfoHdr->bmiHeader.biWidth;
		tmpList[validSize].height = videoInfoHdr->bmiHeader.biHeight;
		tmpList[validSize].fps = 10000000 / videoInfoHdr->AvgTimePerFrame;
		tmpList[validSize].format = videoInfoHdr->bmiHeader.biCompression;

		// 有效长度加1
		validSize++;
	}

	// 拷贝有效列表
	*reply = new VideoFrameInfo[validSize];
	memcpy(*reply, tmpList, validSize * sizeof(VideoFrameInfo));
	*replySize = validSize;
	// 释放原列表
	delete[] tmpList;

	// 释放流配置接口
	pStreamConfig->Release();
	pStreamConfig = nullptr;
	// 释放PIN接口
	pin->Release();
	pin = nullptr;
	// 释放IBaseFilter接口
	pFilter->Release();
	pFilter = nullptr;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备列表
 *
 * @param   reply   响应参数
 * @return  状态码
 */
StatusCode BecamDirectShow::GetDeviceList(GetDeviceListReply* reply) {
	// 检查入参
	if (reply == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 设备数量置空
	reply->deviceInfoListSize = 0;
	reply->deviceInfoList = nullptr;

	// 检查COM库是否初始化成功
	if (!this->comInited) {
		// COM库初始化失败了
		return StatusCode::STATUS_CODE_ERR_INIT_COM;
	}

	// 声明设备枚举器实例
	ICreateDevEnum* pDevEnum = nullptr;
	// 创建设备枚举器
	auto res =
		CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
	if (FAILED(res)) {
		// 创建设备枚举器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_DEVICE_ENUMERATOR;
	}

	// 声明枚举接收器实例
	IEnumMoniker* pEnum = nullptr;
	// 枚举视频捕获设备
	res = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	// 错误码是否为空设备
	if (res == S_FALSE) {
		// 释放设备枚举器
		pDevEnum->Release();
		pDevEnum = nullptr;
		// OK
		return StatusCode::STATUS_CODE_SUCCESS;
	}
	// 其他错误码
	if (FAILED(res)) {
		// 释放设备枚举器
		pDevEnum->Release();
		pDevEnum = nullptr;
		// 设备枚举失败
		return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM;
	}

	// 声明vector来储存设备列表
	std::vector<DeviceInfo> deviceVec;
	// 设备实例
	IMoniker* pMoniker = nullptr;
	// 是否查询到设备
	ULONG pceltFetched;
	// 遍历枚举所有设备
	while (pEnum->Next(1, &pMoniker, &pceltFetched) == S_OK) {
		// 获取设备友好名称
		std::string friendlyName;
		auto code = this->getMonikerFriendlyName(pMoniker, friendlyName);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 释放设备实例
			pMoniker->Release();
			pMoniker = nullptr;
			// 继续遍历
			continue;
		}

		// 获取设备位置信息
		std::string devicePath;
		code = this->getMonikerDevicePath(pMoniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 释放设备实例
			pMoniker->Release();
			pMoniker = nullptr;
			// 继续遍历
			continue;
		}

		// 获取设备视频帧信息
		VideoFrameInfo* list = nullptr;
		size_t listSize = 0;
		auto res = this->getMonikerWithStreamConfig(pMoniker, &list, &listSize);
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			// 释放设备实例
			pMoniker->Release();
			pMoniker = nullptr;
			// 继续遍历
			continue;
		}

		// 赋值提取结果
		DeviceInfo deviceInfo;
		// 转换为C字符串
		deviceInfo.name = new char[friendlyName.size() + 1];
		memcpy(deviceInfo.name, friendlyName.c_str(), friendlyName.size() + 1);
		// 转换为C字符串
		deviceInfo.devicePath = new char[devicePath.size() + 1];
		memcpy(deviceInfo.devicePath, devicePath.c_str(), devicePath.size() + 1);
		// 转换为C字符串
		deviceInfo.locationInfo = nullptr;
		// memcpy(ret, locationInfo.c_str(), locationInfo.size() + 1);
		// 赋值视频帧信息
		deviceInfo.frameInfoListSize = listSize;
		deviceInfo.frameInfoList = list;
		// 追加到结果中
		deviceVec.insert(deviceVec.end(), deviceInfo);

		// 释放当前设备实例
		pMoniker->Release();
		pMoniker = nullptr;
	}

	// 转换为定长数组
	reply->deviceInfoListSize = deviceVec.size();
	reply->deviceInfoList = new DeviceInfo[deviceVec.size()];
	memcpy(reply->deviceInfoList, deviceVec.data(), deviceVec.size() * sizeof(DeviceInfo));

	// 释放设备枚举接收器实例
	pEnum->Release();
	pEnum = nullptr;
	// 释放设备枚举器实例
	pDevEnum->Release();
	pDevEnum = nullptr;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 释放设备列表
 *
 * @param input 输入参数
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
		auto item = &input->deviceInfoList[i];
		// 释放友好名称
		if (item->name != nullptr) {
			delete item->name;
			item->name = nullptr;
		}
		// 释放设备路径
		if (item->devicePath != nullptr) {
			delete item->devicePath;
			item->devicePath = nullptr;
		}
		// 释放位置信息
		if (item->locationInfo != nullptr) {
			delete item->locationInfo;
			item->locationInfo = nullptr;
		}
		// 释放视频帧列表
		if (item->frameInfoListSize > 0 && item->frameInfoList != nullptr) {
			delete item->frameInfoList;
			item->frameInfoListSize = 0;
			item->frameInfoList = nullptr;
		}
	}

	// 释放整个列表
	delete input->deviceInfoList;
	input->deviceInfoListSize = 0;
	input->deviceInfoList = nullptr;
}
