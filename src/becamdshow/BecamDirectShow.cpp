#include "BecamDirectShow.hpp"
#include "BecamAmMediaType.hpp"
#include "BecamMonikerPropReader.hpp"
#include "BecamStringConvert.hpp"

#include <vector>

/**
 * @brief 获取捕获筛选器的输出端口
 *
 * @param	captureFilter	[in]	捕获筛选器实例
 * @param	dir				[in]	输出方向
 * @return	捕获筛选器的输出端口
 */
IPin* BecamDirectShow::getPin(IBaseFilter* captureFilter, PIN_DIRECTION dir) {
	IEnumPins* enumPins;
	if (FAILED(captureFilter->EnumPins(&enumPins)))
		return nullptr;

	IPin* captureOuputPin;
	while (enumPins->Next(1, &captureOuputPin, nullptr) == S_OK) {
		PIN_DIRECTION d;
		captureOuputPin->QueryDirection(&d);
		if (d == dir) {
			enumPins->Release();
			return captureOuputPin;
		}
		captureOuputPin->Release();
	}
	enumPins->Release();
	return nullptr;
}

/**
 * @brief 枚举设备列表
 *
 * @param	callback	[in]	回调函数（回调结束将立即释放IMoniker，回调返回false将立即停止枚举）
 * @return	状态码
 */
StatusCode BecamDirectShow::enumDevices(std::function<bool(IMoniker*)> callback) {
	// 检查COM库是否初始化成功
	if (!this->comInited) {
		// COM库初始化失败了
		return StatusCode::STATUS_CODE_ERR_INIT_COM;
	}

	// 检查回调是否有效
	if (callback == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 声明设备枚举器实例
	ICreateDevEnum* pDevEnum = nullptr;
	// 创建设备枚举器
	auto res =
		CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
	if (FAILED(res)) {
		// 创建设备枚举器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_ENUMERATOR;
	}

	// 声明设备分类枚举器实例
	IEnumMoniker* pEnum = nullptr;
	// 枚举视频捕获设备
	res = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	// 错误码是否为空设备
	if (FAILED(res) || res == S_FALSE) {
		// 释放设备枚举器
		pDevEnum->Release();
		pDevEnum = nullptr;
		// 分类枚举器创建成功了，但是没有设备
		if (res == S_FALSE) {
			// OK
			return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
		}
		// 设备枚举失败
		return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM;
	}

	// 设备实例
	IMoniker* moniker = nullptr;
	// 是否查询到设备
	ULONG _pceltFetched = 0;
	// 遍历枚举所有设备
	while (pEnum->Next(1, &moniker, &_pceltFetched) == S_OK) {
		// 回调枚举出来的设备
		auto cbRes = callback(moniker);
		// 释放设备实例
		moniker->Release();
		moniker = nullptr;
		// 是否需要停止枚举
		if (!cbRes) {
			break;
		}
	}

	// 释放设备分类枚举器
	pEnum->Release();
	pEnum = nullptr;
	// 释放设备枚举器
	pDevEnum->Release();
	pDevEnum = nullptr;

	// 枚举结束
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备友好名称
 *
 * @param	moniker		[in]	设备实例
 * @param	devicePath	[out]	设备友好名称
 * @return	状态码
 */
StatusCode BecamDirectShow::getMonikerFriendlyName(IMoniker* moniker, std::string& friendlyName) {
	// 检查参数
	if (moniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = BecamMonikerPropReader(L"FriendlyName");
	auto res = reader.read(moniker);
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

/**
 * @brief 获取设备路径
 *
 * @param	moniker		[in]	设备实例
 * @param	devicePath	[out]	设备路径
 * @return	状态码
 */
StatusCode BecamDirectShow::getMonikerDevicePath(IMoniker* moniker, std::string& devicePath) {
	// 检查参数
	if (moniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = BecamMonikerPropReader(L"DevicePath");
	auto res = reader.read(moniker);
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

/**
 * @brief 获取设备
 * @param	devicePath	[in]	设备路径
 * @param	moniker		[out]	设备实例
 * @return	状态码
 */
StatusCode BecamDirectShow::getDevice(const std::string devicePath, IMoniker*& moniker) {
	// 回调中的错误码
	StatusCode errCode = StatusCode::STATUS_CODE_SUCCESS;
	// 枚举设备
	auto code = this->enumDevices([this, devicePath, &moniker, &errCode](IMoniker* tmpmMoniker) {
		// 获取设备路径
		std::string tmpDevicePath = "";
		errCode = this->getMonikerDevicePath(tmpmMoniker, tmpDevicePath);
		if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
			// 终止枚举
			return false;
		}

		// 检查设备路径是否一致
		if (devicePath == tmpDevicePath) {
			// 导出获取到的设备实例
			moniker = tmpmMoniker;
			// 增加一个引用计数
			moniker->AddRef();
			// 终止枚举
			return false;
		}

		// 继续枚举
		return true;
	});

	// 是否异常
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备支持的流能力
 *
 * @param	streamConfig	[in]	设备流配置实例
 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return	状态码
 */
StatusCode BecamDirectShow::getDeviceStreamCaps(IAMStreamConfig* streamConfig,
												std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (streamConfig == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 声明用来接收支持的流能力总数量
	int count, size;
	// 获取流能力总数量
	auto res = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(res)) {
		// 获取流能力总数量失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 遍历所有流配置
	for (int i = 0; i < count; ++i) {
		// 获取流配置信息
		AM_MEDIA_TYPE* pmt = NULL;
		VIDEO_STREAM_CONFIG_CAPS scc;
		res = streamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(res)) {
			// 分析失败，继续下一个
			continue;
		}

		// 检查一下
		if (pmt->majortype != MEDIATYPE_Video || pmt->formattype != FORMAT_VideoInfo || pmt->pbFormat == nullptr) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			pmt = nullptr;
			// 信息不是想要的
			continue;
		}

		// 是否需要停止枚举
		if (!callback(pmt)) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			pmt = nullptr;
			break;
		}

		// 释放AM_MEDIA_TYPE
		_DeleteMediaType(pmt);
		pmt = nullptr;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备支持的流能力
 *
 * @param	captureOuputPin	[in]	捕获筛选器的输出端口
 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return	状态码
 */
StatusCode BecamDirectShow::getDeviceStreamCaps(IPin* captureOuputPin, std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (captureOuputPin == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 获取流配置
	IAMStreamConfig* streamConfig = nullptr;
	auto res = captureOuputPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
	if (FAILED(res)) {
		// 获取流配置失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 调用重载方法
	auto code = getDeviceStreamCaps(streamConfig, callback);

	// 释放设备流配置
	streamConfig->Release();
	streamConfig = nullptr;

	// 返回结果
	return code;
}

/**
 * @brief 枚举设备支持的流能力
 *
 * @param	captureFilter	[in]	捕获筛选器实例
 * @param	callback		[in]	回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return	状态码
 */
StatusCode BecamDirectShow::getDeviceStreamCaps(IBaseFilter* captureFilter,
												std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (captureFilter == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 获取捕获筛选器的输出端点
	auto captureOuputPin = this->getPin(captureFilter, PINDIR_OUTPUT);
	if (captureOuputPin == nullptr) {
		// 获取捕获筛选器的输出端点失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 调用重载函数
	auto code = getDeviceStreamCaps(captureOuputPin, callback);

	// 释放捕获筛选器的输出端点
	captureOuputPin->Release();
	captureOuputPin = nullptr;

	// 返回结果
	return code;
}

/**
 * @brief 获取设备支持的流能力
 *
 * @param	moniker		[in]	设备实例
 * @param	reply		[out]	设备流能力列表
 * @param	replySize	[out]	设备流能力列表大小
 * @return	状态码
 */
StatusCode BecamDirectShow::getDeviceStreamCaps(IMoniker* moniker, VideoFrameInfo** reply, size_t* replySize) {
	// 检查参数
	if (moniker == nullptr || reply == nullptr || replySize == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 声明捕获筛选器实例
	IBaseFilter* captureFilter = nullptr;
	// 绑定到捕获筛选器实例
	auto res = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&captureFilter);
	// 检查是否绑定成功
	if (FAILED(res)) {
		// 绑定设备实例失败
		return StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
	}

	// 置零
	*reply = nullptr;
	*replySize = 0;
	// 声明vector
	std::vector<VideoFrameInfo> replyVec;

	// 执行流能力枚举
	auto code = this->getDeviceStreamCaps(captureFilter, [&replyVec](AM_MEDIA_TYPE* pmt) {
		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		// 构建信息
		VideoFrameInfo info = {0};
		info.width = videoInfoHdr->bmiHeader.biWidth;		 // 提取宽度
		info.height = videoInfoHdr->bmiHeader.biHeight;		 // 提取高度
		info.fps = 10000000 / videoInfoHdr->AvgTimePerFrame; // 提取帧率
		info.format = videoInfoHdr->bmiHeader.biCompression; // 提取格式
		// 插入到列表中
		replyVec.insert(replyVec.end(), info);
		// 始终继续查找下一个
		return true;
	});

	// 释放捕获筛选器实例
	captureFilter->Release();
	captureFilter = nullptr;

	// 是否失败了
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败了
		return code;
	}

	// 是否查询到有效流能力
	if (replyVec.size() > 0) {
		// 拷贝有效列表
		*replySize = replyVec.size();
		*reply = new VideoFrameInfo[replyVec.size()];
		memcpy(*reply, replyVec.data(), replyVec.size() * sizeof(VideoFrameInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 设置设备支持的流能力
 *
 * @param	captureOuputPin	[in]	捕获筛选器的输出端点
 * @param	frameInfo		[in]	视频帧信息
 * @return	状态码
 */
StatusCode BecamDirectShow::setCaptureOuputPinStreamCaps(IPin* captureOuputPin, const VideoFrameInfo frameInfo) {
	// 检查参数
	if (captureOuputPin == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 获取流配置
	IAMStreamConfig* streamConfig = nullptr;
	auto res = captureOuputPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
	if (FAILED(res)) {
		// 获取流配置失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 回调中的状态（默认为未找到对应的流能力信息）
	auto errCode = StatusCode::STATUS_CODE_ERR_NOMATCH_STREAM_CAPS;
	// 遍历设备所有流能力
	auto code = this->getDeviceStreamCaps(streamConfig, [streamConfig, frameInfo, &errCode](AM_MEDIA_TYPE* pmt) {
		// 检查一下
		if (pmt->majortype != MEDIATYPE_Video || pmt->formattype != FORMAT_VideoInfo || pmt->pbFormat == nullptr) {
			// 信息不是想要的
			return true;
		}

		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		auto width = videoInfoHdr->bmiHeader.biWidth;		 // 提取宽度
		auto height = videoInfoHdr->bmiHeader.biHeight;		 // 提取高度
		auto fps = 10000000 / videoInfoHdr->AvgTimePerFrame; // 提取帧率
		auto format = videoInfoHdr->bmiHeader.biCompression; // 提取格式
		// 信息是否一致
		if (frameInfo.width == width && frameInfo.height == height && frameInfo.fps == fps &&
			frameInfo.format == format) {
			// 设定媒体类型（包括分辨率，帧率，格式）
			auto res = streamConfig->SetFormat(pmt);
			if (SUCCEEDED(res)) {
				// 设定成功
				errCode = StatusCode::STATUS_CODE_SUCCESS;
			} else {
				// 设定失败
				errCode = StatusCode::STATUS_CODE_ERR_SET_MEDIA_TYPE;
			}
			// 不管是否成功都要结束枚举
			return false;
		}

		// 继续找下一个
		return true;
	});

	// 释放设备流配置
	streamConfig->Release();
	streamConfig = nullptr;

	// 检查结果
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备列表
 *
 * @param	reply	[out]	响应参数
 * @return	状态码
 */
StatusCode BecamDirectShow::GetDeviceList(GetDeviceListReply* reply) {
	// 检查入参
	if (reply == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 设备数量置空
	reply->deviceInfoListSize = 0;
	reply->deviceInfoList = nullptr;

	// 声明vector来储存设备列表
	std::vector<DeviceInfo> deviceVec;
	// 开始枚举设备
	auto code = this->enumDevices([this, &deviceVec](IMoniker* moniker) {
		// 获取设备友好名称
		std::string friendlyName = "";
		auto code = this->getMonikerFriendlyName(moniker, friendlyName);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备路径
		std::string devicePath = "";
		code = this->getMonikerDevicePath(moniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备位置信息
		std::string locationInfo = "";

		// 获取设备流能力
		VideoFrameInfo* list = nullptr;
		size_t listSize = 0;
		code = this->getDeviceStreamCaps(moniker, &list, &listSize);
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
 * @param	input	[in]	输入参数
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
 * @param	devicePath	[in]	设备路径
 * @param	frameInfo	[in]	设置的视频帧信息
 * @return 状态码
 */
StatusCode BecamDirectShow::OpenDevice(const std::string devicePath, const VideoFrameInfo* frameInfo) {
	// 检查入参
	if (devicePath.empty() || frameInfo == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}
	if (frameInfo->width <= 0 || frameInfo->height <= 0 || frameInfo->fps <= 0 || frameInfo->format <= 0) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 声明设备实例
	IMoniker* moniker;
	// 筛选设备
	auto code = this->getDevice(devicePath, moniker);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}

	// 声明捕获筛选器实例
	IBaseFilter* captureFilter = nullptr;
	// 绑定到捕获筛选器实例
	auto res = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&captureFilter);
	// 释放设备实例
	moniker->Release();
	// 检查是否绑定成功
	if (FAILED(res)) {
		// 绑定设备实例失败
		return StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
	}

	// 获取捕获筛选器的输出端口
	auto captureOuputPin = this->getPin(captureFilter, PINDIR_OUTPUT);
	if (captureOuputPin == nullptr) {
		// 释放捕获筛选器实例
		captureFilter->Release();
		captureFilter = nullptr;
		// 获取PIN接口失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 配置设备流能力
	code = this->setCaptureOuputPinStreamCaps(captureOuputPin, *frameInfo);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 释放捕获筛选器的输出端口
		captureOuputPin->Release();
		captureOuputPin = nullptr;
		// 释放捕获筛选器实例
		captureFilter->Release();
		captureFilter = nullptr;
		// 配置设备流能力失败
		return code;
	}

	// 关闭已打开的设备
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}

	// 尝试打开设备
	this->openedDevice = new BecamOpenedDevice();
	code = this->openedDevice->Open(captureFilter, captureOuputPin);

	// 释放捕获筛选器的输出端口
	captureOuputPin->Release();
	captureOuputPin = nullptr;
	// 释放捕获筛选器
	captureFilter->Release();
	captureFilter = nullptr;

	// 是否打开成功
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 打开失败，关闭实例
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}

	// 返回结果
	return code;
}

/**
 * @brief 关闭设备
 */
void BecamDirectShow::CloseDevice() {
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
 * @param	data	视频帧流
 * @param	size	视频帧流大小
 * @return	状态码
 */
StatusCode BecamDirectShow::GetFrame(uint8_t** data, size_t* size) {
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
 * @param	data	视频帧流
 */
void BecamDirectShow::FreeFrame(uint8_t** data) {
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
