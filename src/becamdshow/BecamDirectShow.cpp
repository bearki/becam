#include "BecamDirectShow.hpp"
#include "MonikerPropReader.hpp"
#include "StringConvert.hpp"

#include <vector>

// -------------------- 重写Dshow函数，在新开发环境上不提供了 -------------------- //

// Release the format block for a media type.
void _FreeMediaType(AM_MEDIA_TYPE& mt) {
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

// Delete a media type structure that was allocated on the heap.
void _DeleteMediaType(AM_MEDIA_TYPE* pmt) {
	if (pmt != NULL) {
		_FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

// -------------------------- 实现BecamDirectShow类 -------------------------- //

/**
 * @brief 枚举设备列表
 *
 * @param callback 回调函数（回调完成IMoniker将立即释放，回调返回false将立即停止枚举）
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
	IMoniker* pMoniker = nullptr;
	// 是否查询到设备
	ULONG pceltFetched = 0;
	// 遍历枚举所有设备
	while (pEnum->Next(1, &pMoniker, &pceltFetched) == S_OK) {
		// 回调枚举出来的设备
		auto exitEnum = callback(pMoniker);
		// 释放设备实例
		pMoniker->Release();
		// 是否需要 "停止枚举"
		if (!exitEnum) {
			// 跳出
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
StatusCode BecamDirectShow::getMonikerWithStreamConfig(IMoniker* pMoniker, const VideoFrameInfo* filter,
													   VideoFrameInfo** reply, size_t* replySize) {
	// 检查参数
	if (pMoniker == nullptr || reply == nullptr || replySize == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 置零
	*reply = nullptr;
	*replySize = 0;

	// 绑定到IBaseFilter接口
	// TODO: 目前BindToObject会发生内存泄漏，没搞懂咋回事，有空再弄吧
	IBaseFilter* pFilter = nullptr;
	auto res = pMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pFilter);
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
		// 获取流配置信息
		AM_MEDIA_TYPE* pmt = NULL;
		VIDEO_STREAM_CONFIG_CAPS scc;
		res = pStreamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(res)) {
			// 分析失败，继续下一个
			continue;
		}
		if (pmt->majortype != MEDIATYPE_Video || pmt->formattype != FORMAT_VideoInfo || pmt->pbFormat == nullptr) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			// 信息不是想要的
			continue;
		}

		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		auto width = videoInfoHdr->bmiHeader.biWidth;
		auto height = videoInfoHdr->bmiHeader.biHeight;
		auto fps = 10000000 / videoInfoHdr->AvgTimePerFrame;
		auto format = videoInfoHdr->bmiHeader.biCompression;
		// 释放AM_MEDIA_TYPE
		_DeleteMediaType(pmt);

		// 是否执行了筛选
		if (filter != nullptr) {
			// 是否符合筛选结果
			if (width == filter->width && height == filter->height && fps == filter->fps && format == filter->format) {
				// 赋值视频帧信息
				tmpList[validSize].width = width;
				tmpList[validSize].height = height;
				tmpList[validSize].fps = fps;
				tmpList[validSize].format = format;
				// 有效长度加1
				validSize++;
				// 跳出
				break;
			}
		} else {
			// 赋值视频帧信息
			tmpList[validSize].width = width;
			tmpList[validSize].height = height;
			tmpList[validSize].fps = fps;
			tmpList[validSize].format = format;
			// 有效长度加1
			validSize++;
		}
	}

	// 释放流配置接口
	pStreamConfig->Release();
	pStreamConfig = nullptr;
	// 释放PIN接口
	pin->Release();
	pin = nullptr;
	// 释放IBaseFilter接口
	pFilter->Release();
	pFilter = nullptr;

	// 检查下是否未筛选到设备
	if (filter != nullptr && validSize <= 0) {
		// 返回错误
		return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
	}

	if (validSize > 0) {
		// 拷贝有效列表
		*reply = new VideoFrameInfo[validSize];
		memcpy(*reply, tmpList, validSize * sizeof(VideoFrameInfo));
		*replySize = validSize;
		// 释放原列表
		delete tmpList;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备列表
 *
 * @param reply 响应参数
 * @return 状态码
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
	auto code = this->enumDevices([this, &deviceVec](IMoniker* pMoniker) {
		// 获取设备友好名称
		std::string friendlyName = "";
		auto code = this->getMonikerFriendlyName(pMoniker, friendlyName);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备路径
		std::string devicePath = "";
		code = this->getMonikerDevicePath(pMoniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return true;
		}

		// 获取设备位置信息
		std::string locationInfo = "";

		// 获取设备视频帧信息
		VideoFrameInfo* list = nullptr;
		size_t listSize = 0;
		code = this->getMonikerWithStreamConfig(pMoniker, nullptr, &list, &listSize);
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

/**
 * @brief 打开指定设备
 *
 * @param devicePath 设备路径
 * @param frameInfo 设置的视频帧信息
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

	// 声明捕获筛选器实例
	IBaseFilter* pCaptureFilter = nullptr;
	// 预声明绑定结果
	HRESULT res = S_OK;
	// 回调中发生的错误码
	auto callbackCode = StatusCode::STATUS_CODE_SUCCESS;
	// 枚举设备，直到找到合适的设备
	auto code =
		this->enumDevices([this, devicePath, frameInfo, &res, &pCaptureFilter, &callbackCode](IMoniker* pMoniker) {
			// 获取设备路径
			std::string tmpDevicePath = "";
			callbackCode = this->getMonikerDevicePath(pMoniker, tmpDevicePath);
			if (callbackCode != StatusCode::STATUS_CODE_SUCCESS) {
				// 失败了
				return false;
			}
			// 是否是这个设备
			if (tmpDevicePath != devicePath) {
				// 不是这个设备，继续查
				return true;
			}

			// 获取设备视频帧信息
			VideoFrameInfo* list = nullptr;
			size_t size = 0;
			callbackCode = this->getMonikerWithStreamConfig(pMoniker, frameInfo, &list, &size);
			if (callbackCode != StatusCode::STATUS_CODE_SUCCESS) {
				// 失败了
				return false;
			}

			// 绑定到捕获筛选器
			res = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pCaptureFilter);
			if (FAILED(res)) {
				// 绑定设备实例事变
				callbackCode = StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
				// 失败了
				return false;
			}

			// 找到后立即结束枚举
			return false;
		});

	// 检查设备枚举状态码
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		return code;
	}
	// 检查设备枚举回调错误码
	if (callbackCode != StatusCode::STATUS_CODE_SUCCESS) {
		return callbackCode;
	}

	// 关闭已打开的设备
	if (this->openedDevice != nullptr) {
		delete this->openedDevice;
		this->openedDevice = nullptr;
	}

	// 尝试打开设备
	this->openedDevice = new BecamOpenedDevice(pCaptureFilter);
	code = this->openedDevice->Open();
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 打开失败，关闭实例
		delete this->openedDevice;
		return code;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}
