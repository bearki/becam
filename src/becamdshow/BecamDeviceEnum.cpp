
#include "BecamDeviceEnum.hpp"
#include "BecamAmMediaType.hpp"
#include "BecamMonikerPropReader.hpp"
#include <pkg/LogOutput.hpp>
#include <pkg/StringConvert.hpp>
#include <vector>

/**
 * @implements 实现构造函数
 */
BecamDeviceEnum::BecamDeviceEnum(bool initCom) {
	// 是否需要初始化COM库
	if (initCom) {
		// 初始化COM库（每个线程都需要单独初始化一下COM库）
		auto res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(res)) {
			// COM库初始化失败了
			this->comInited = false;
			DEBUG_LOG("COM library init failed, HRESULT: " << res);
		} else {
			// COM库初始化成功了
			this->comInited = true;
		}
	}
}

/**
 * @implements 实现析构函数
 */
BecamDeviceEnum::~BecamDeviceEnum() {
	// 释放视频输入设备枚举器实例
	if (this->videoInputEnumInstance != nullptr) {
		// 释放视频输入设备枚举器实例
		this->videoInputEnumInstance->Release();
		// 重置
		this->videoInputEnumInstance = nullptr;
	}
	// 释放设备枚举器实例
	if (this->devEnumInstance != nullptr) {
		// 释放设备枚举器实例
		this->devEnumInstance->Release();
		// 重置
		this->devEnumInstance = nullptr;
	}
	// 是否需要释放COM库
	if (this->comInited) {
		// 释放COM库
		CoUninitialize();
		// 重置掉标识
		this->comInited = false;
	}
}

/**
 * @implements 实现枚举所有视频设备
 */
StatusCode BecamDeviceEnum::EnumVideoDevices(std::function<bool(IMoniker*)> callback) {
	// 检查回调是否有效
	if (callback == nullptr) {
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 声明响应
	HRESULT res;

	// 创建设备枚举器
	res = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&this->devEnumInstance);
	if (FAILED(res)) {
		// 创建设备枚举器失败
		DEBUG_LOG("BecamDeviceEnum::EnumVideoDevices -> CoCreateInstance(CLSID_SystemDeviceEnum) failed, HRESULT: " << res);
		return StatusCode::STATUS_CODE_DSHOW_ERR_CREATE_ENUMERATOR;
	}

	// 枚举视频捕获设备
	res = this->devEnumInstance->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &this->videoInputEnumInstance, 0);
	// 错误码是否为空设备
	if (FAILED(res) || res == S_FALSE) {
		// 分类枚举器创建成功了，但是没有设备
		if (res == S_FALSE) {
			// OK
			return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_FOUND;
		}
		// 设备枚举失败
		DEBUG_LOG("BecamDeviceEnum::EnumVideoDevices -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory) "
				  "failed, HRESULT: "
				  << res);
		return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM_FAILED;
	}

	// 设备实例
	IMoniker* moniker = nullptr;
	// 是否查询到设备
	ULONG _pceltFetched = 0;
	// 遍历枚举所有设备
	while (this->videoInputEnumInstance->Next(1, &moniker, &_pceltFetched) == S_OK) {
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

	// 枚举结束
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取设备友好名称
 */
StatusCode BecamDeviceEnum::GetMonikerFriendlyName(IMoniker* moniker, std::string& friendlyName) {
	// 检查参数
	if (moniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = BecamMonikerPropReader(L"FriendlyName");
	auto res = reader.read(moniker);
	if (res.first != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败
		DEBUG_LOG("BecamDeviceEnum::GetMonikerFriendlyName -> read failed, CODE: " << res.first);
		return res.first;
	}

	// UTF16转换为UTF8
	friendlyName = WStringToString(res.second.bstrVal);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取设备路径
 */
StatusCode BecamDeviceEnum::GetMonikerDevicePath(IMoniker* moniker, std::string& devicePath) {
	// 检查参数
	if (moniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 构建阅读器并读取属性值
	auto reader = BecamMonikerPropReader(L"DevicePath");
	auto res = reader.read(moniker);
	if (res.first != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败
		DEBUG_LOG("BecamDeviceEnum::GetMonikerDevicePath -> read failed, CODE: " << res.first);
		return res.first;
	}

	// UTF16转换为UTF8
	devicePath = WStringToString(res.second.bstrVal);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取设备引用
 */
StatusCode BecamDeviceEnum::GetDeviceRef(const std::string devicePath, IMoniker*& moniker) {
	// 回调中的错误码
	StatusCode errCode = StatusCode::STATUS_CODE_SUCCESS;
	// 枚举设备
	auto code = this->EnumVideoDevices([this, devicePath, &moniker, &errCode](IMoniker* tmpmMoniker) {
		// 获取设备路径
		std::string tmpDevicePath = "";
		errCode = this->GetMonikerDevicePath(tmpmMoniker, tmpDevicePath);
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
		DEBUG_LOG("BecamDeviceEnum::GetDeviceRef -> EnumVideoDevices failed, CODE: " << code);
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		DEBUG_LOG("BecamDeviceEnum::GetDeviceRef -> EnumVideoDevices callback failed, CODE: " << errCode);
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取捕获筛选器的输出端口
 */
IPin* BecamDeviceEnum::GetPin(IBaseFilter* captureFilter, PIN_DIRECTION dir) {
	IEnumPins* enumPins;
	auto res = captureFilter->EnumPins(&enumPins);
	if (FAILED(res)) {
		DEBUG_LOG("BecamDeviceEnum::GetPin -> EnumPins failed, HRESULT: " << res);
		return nullptr;
	}

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
 * @implements 实现获取设备支持的流能力
 */
StatusCode BecamDeviceEnum::GetDeviceStreamCaps(IAMStreamConfig* streamConfig, std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (streamConfig == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 声明用来接收支持的流能力总数量
	int count, size;
	// 获取流能力总数量
	auto res = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(res)) {
		// 获取流能力总数量失败
		DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> GetNumberOfCapabilities failed, HRESULT: " << res);
		return StatusCode::STATUS_CODE_DSHOW_ERR_GET_STREAM_CAPS;
	}

	// 遍历所有流配置
	for (int i = 0; i < count; ++i) {
		// 获取流配置信息
		AM_MEDIA_TYPE* pmt = NULL;
		VIDEO_STREAM_CONFIG_CAPS scc;
		res = streamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(res)) {
			// 分析失败，继续下一个
			DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> GetStreamCaps failed, HRESULT: " << res);
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
 * @implements 实现获取设备支持的流能力
 */
StatusCode BecamDeviceEnum::GetDeviceStreamCaps(IPin* captureOuputPin, std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (captureOuputPin == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 获取流配置
	IAMStreamConfig* streamConfig = nullptr;
	auto res = captureOuputPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
	if (FAILED(res)) {
		// 获取流配置失败
		DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> QueryInterface(IID_IAMStreamConfig) failed, HRESULT: " << res);
		return StatusCode::STATUS_CODE_DSHOW_ERR_GET_STREAM_CAPS;
	}

	// 调用重载方法
	auto code = BecamDeviceEnum::GetDeviceStreamCaps(streamConfig, callback);

	// 释放设备流配置
	streamConfig->Release();
	streamConfig = nullptr;

	// 返回结果
	return code;
}

/**
 * @implements 实现枚举设备支持的流能力
 */
StatusCode BecamDeviceEnum::GetDeviceStreamCaps(IBaseFilter* captureFilter, std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (captureFilter == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 获取捕获筛选器的输出端点
	auto captureOuputPin = BecamDeviceEnum::GetPin(captureFilter, PINDIR_OUTPUT);
	if (captureOuputPin == nullptr) {
		// 获取捕获筛选器的输出端点失败
		DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> GetPin failed");
		return StatusCode::STATUS_CODE_DSHOW_ERR_GET_STREAM_CAPS;
	}

	// 调用重载函数
	auto code = BecamDeviceEnum::GetDeviceStreamCaps(captureOuputPin, callback);

	// 释放捕获筛选器的输出端点
	captureOuputPin->Release();
	captureOuputPin = nullptr;

	// 返回结果
	return code;
}

/**
 * @implements 实现获取设备支持的流能力
 */
StatusCode BecamDeviceEnum::GetDeviceStreamCaps(IMoniker* moniker, VideoFrameInfo*& reply, size_t& replySize) {
	// 检查参数
	if (moniker == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 声明捕获筛选器实例
	IBaseFilter* captureFilter = nullptr;
	// 绑定到捕获筛选器实例
	auto res = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&captureFilter);
	// 检查是否绑定成功
	if (FAILED(res)) {
		// 绑定设备实例失败
		DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> BindToObject failed, HRESULT: " << res);
		return StatusCode::STATUS_CODE_ERR_DEVICE_OPEN_FAILED;
	}

	// 置零
	reply = nullptr;
	replySize = 0;
	// 声明vector
	std::vector<VideoFrameInfo> replyVec;

	// 执行流能力枚举
	auto code = BecamDeviceEnum::GetDeviceStreamCaps(captureFilter, [&replyVec](AM_MEDIA_TYPE* pmt) {
		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		// 构建信息
		VideoFrameInfo info = {0};
		info.width = videoInfoHdr->bmiHeader.biWidth;		 // 提取宽度
		info.height = videoInfoHdr->bmiHeader.biHeight;		 // 提取高度
		auto fps = 10000000 / videoInfoHdr->AvgTimePerFrame; // 提取帧率
		info.fps = static_cast<uint32_t>(fps);
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
		DEBUG_LOG("BecamDeviceEnum::GetDeviceStreamCaps -> GetDeviceStreamCaps failed, CODE: " << code);
		return code;
	}

	// 是否查询到有效流能力
	if (replyVec.size() > 0) {
		// 拷贝有效列表
		replySize = replyVec.size();
		reply = new VideoFrameInfo[replySize];
		memcpy(reply, replyVec.data(), replySize * sizeof(VideoFrameInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现设置设备支持的流能力
 */
StatusCode BecamDeviceEnum::SetCaptureOuputPinStreamCaps(IPin* captureOuputPin, const VideoFrameInfo frameInfo) {
	// 检查参数
	if (captureOuputPin == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM;
	}

	// 获取流配置
	IAMStreamConfig* streamConfig = nullptr;
	auto res = captureOuputPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
	if (FAILED(res)) {
		// 获取流配置失败
		DEBUG_LOG("BecamDeviceEnum::SetCaptureOuputPinStreamCaps -> QueryInterface(IID_IAMStreamConfig) failed, HRESULT: " << res);
		return StatusCode::STATUS_CODE_DSHOW_ERR_GET_STREAM_CAPS;
	}

	// 回调中的状态（默认为未找到对应的流能力信息）
	auto errCode = StatusCode::STATUS_CODE_ERR_DEVICE_FRAME_FMT_NOT_FOUND;
	// 遍历设备所有流能力
	auto code = BecamDeviceEnum::GetDeviceStreamCaps(streamConfig, [streamConfig, frameInfo, &errCode](AM_MEDIA_TYPE* pmt) {
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
		if (frameInfo.width == width && frameInfo.height == height && frameInfo.fps == fps && frameInfo.format == format) {
			// 设定媒体类型（包括分辨率，帧率，格式）
			auto res = streamConfig->SetFormat(pmt);
			if (SUCCEEDED(res)) {
				// 设定成功
				errCode = StatusCode::STATUS_CODE_SUCCESS;
			} else {
				// 设定失败
				DEBUG_LOG("BecamDeviceEnum::SetCaptureOuputPinStreamCaps -> SetFormat failed, HRESULT: " << res);
				errCode = StatusCode::STATUS_CODE_ERR_DEVICE_FRAME_FMT_SET_FAILED;
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
		DEBUG_LOG("BecamDeviceEnum::SetCaptureOuputPinStreamCaps -> GetDeviceStreamCaps failed, CODE: " << code);
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		DEBUG_LOG("BecamDeviceEnum::SetCaptureOuputPinStreamCaps -> GetDeviceStreamCaps callback failed, CODE: " << errCode);
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}
