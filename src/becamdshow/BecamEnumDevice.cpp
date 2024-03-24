
#include "BecamEnumDevice.hpp"
#include "BecamAmMediaType.hpp"
#include "BecamMonikerPropReader.hpp"
#include "BecamStringConvert.hpp"
#include <iostream>

/**
 * @brief 构造函数
 */
BecamEnumDevice::BecamEnumDevice(bool initCom) {
	// 是否需要初始化COM库
	if (initCom) {
		// 初始化COM库
		auto res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(res)) {
			// COM库初始化失败了
			this->comInited = false;
			std::cerr << "COM library init failed, HRESULT: " << res << std::endl;
		} else {
			// COM库初始化成功了
			this->comInited = true;
		}
	}
}

/**
 * @brief 析构函数
 */
BecamEnumDevice::~BecamEnumDevice() {
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
 * @brief 枚举所有视频设备
 *
 * @param callback [in] 回调函数（回调结束将立即释放IMoniker，回调返回false将立即停止枚举）
 * @return 状态码
 */
StatusCode BecamEnumDevice::EnumVideoDevices(std::function<bool(IMoniker*)> callback) {
	// 检查回调是否有效
	if (callback == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 声明响应
	HRESULT res;

	// 创建设备枚举器
	res = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
						   (void**)&this->devEnumInstance);
	if (FAILED(res)) {
		// 创建设备枚举器失败
		std::cerr << "BecamEnumDevice::EnumVideoDevices -> CoCreateInstance(CLSID_SystemDeviceEnum) failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_CREATE_ENUMERATOR;
	}

	// 枚举视频捕获设备
	res =
		this->devEnumInstance->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &this->videoInputEnumInstance, 0);
	// 错误码是否为空设备
	if (FAILED(res) || res == S_FALSE) {
		// 分类枚举器创建成功了，但是没有设备
		if (res == S_FALSE) {
			// OK
			return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
		}
		// 设备枚举失败
		std::cerr << "BecamEnumDevice::EnumVideoDevices -> CreateClassEnumerator(CLSID_VideoInputDeviceCategory) "
					 "failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM;
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
 * @brief 获取设备友好名称
 *
 * @param moniker [in] 设备实例
 * @param friendlyName [out] 设备友好名称
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetMonikerFriendlyName(IMoniker* moniker, std::string& friendlyName) {
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
		std::cerr << "BecamEnumDevice::GetMonikerFriendlyName -> read failed, CODE: " << res.first << std::endl;
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
 * @param moniker [in] 设备实例
 * @param devicePath [out] 设备路径
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetMonikerDevicePath(IMoniker* moniker, std::string& devicePath) {
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
		std::cerr << "BecamEnumDevice::GetMonikerDevicePath -> read failed, CODE: " << res.first << std::endl;
		return res.first;
	}

	// UTF16转换为UTF8
	// 一般可以加一下第二个参数，顺便切换编码
	devicePath = WcharToChar(res.second.bstrVal, CP_UTF8);

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备引用
 * @param devicePath [in] 设备路径
 * @param moniker [out] 设备实例
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetDeviceRef(std::string devicePath, IMoniker*& moniker) {
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
		std::cerr << "BecamEnumDevice::GetDeviceRef -> EnumVideoDevices failed, CODE: " << code << std::endl;
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "BecamEnumDevice::GetDeviceRef -> EnumVideoDevices callback failed, CODE: " << errCode
				  << std::endl;
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取捕获筛选器的输出端口
 *
 * @param captureFilter [in] 捕获筛选器实例
 * @param dir [in] 输出方向
 * @return 捕获筛选器的输出端口
 */
IPin* BecamEnumDevice::GetPin(IBaseFilter* captureFilter, PIN_DIRECTION dir) {
	IEnumPins* enumPins;
	auto res = captureFilter->EnumPins(&enumPins);
	if (FAILED(res)) {
		std::cerr << "BecamEnumDevice::GetPin -> EnumPins failed, HRESULT: " << res << std::endl;
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
 * @brief 获取设备支持的流能力
 *
 * @param streamConfig [in] 设备流配置实例
 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetDeviceStreamCaps(IAMStreamConfig* streamConfig,
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
		std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> GetNumberOfCapabilities failed, HRESULT: " << res
				  << std::endl;
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
			std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> GetStreamCaps failed, HRESULT: " << res << std::endl;
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
 * @param captureOuputPin [in] 捕获筛选器的输出端点
 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetDeviceStreamCaps(IPin* captureOuputPin, std::function<bool(AM_MEDIA_TYPE*)> callback) {
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
		std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> QueryInterface(IID_IAMStreamConfig) failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 调用重载方法
	auto code = BecamEnumDevice::GetDeviceStreamCaps(streamConfig, callback);

	// 释放设备流配置
	streamConfig->Release();
	streamConfig = nullptr;

	// 返回结果
	return code;
}

/**
 * @brief 枚举设备支持的流能力
 *
 * @param captureFilter [in] 捕获筛选器实例
 * @param callback [in] 回调函数（回调结束将立即释放AM_MEDIA_TYPE，回调返回false将立即停止枚举）
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetDeviceStreamCaps(IBaseFilter* captureFilter,
												std::function<bool(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (captureFilter == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 获取捕获筛选器的输出端点
	auto captureOuputPin = BecamEnumDevice::GetPin(captureFilter, PINDIR_OUTPUT);
	if (captureOuputPin == nullptr) {
		// 获取捕获筛选器的输出端点失败
		std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> GetPin failed" << std::endl;
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 调用重载函数
	auto code = BecamEnumDevice::GetDeviceStreamCaps(captureOuputPin, callback);

	// 释放捕获筛选器的输出端点
	captureOuputPin->Release();
	captureOuputPin = nullptr;

	// 返回结果
	return code;
}

/**
 * @brief 获取设备支持的流能力
 *
 * @param moniker [in]设备实例
 * @param reply [out] 设备流能力列表
 * @param replySize [out] 设备流能力列表大小
 * @return 状态码
 */
StatusCode BecamEnumDevice::GetDeviceStreamCaps(IMoniker* moniker, VideoFrameInfo** reply, size_t* replySize) {
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
		std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> BindToObject failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
	}

	// 置零
	*reply = nullptr;
	*replySize = 0;
	// 声明vector
	std::vector<VideoFrameInfo> replyVec;

	// 执行流能力枚举
	auto code = BecamEnumDevice::GetDeviceStreamCaps(captureFilter, [&replyVec](AM_MEDIA_TYPE* pmt) {
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
		std::cerr << "BecamEnumDevice::GetDeviceStreamCaps -> GetDeviceStreamCaps failed, CODE: " << code << std::endl;
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
 * @param captureOuputPin [in] 捕获筛选器的输出端点
 * @param frameInfo [in] 视频帧信息
 * @return 状态码
 */
StatusCode BecamEnumDevice::SetCaptureOuputPinStreamCaps(IPin* captureOuputPin, const VideoFrameInfo frameInfo) {
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
		std::cerr
			<< "BecamEnumDevice::SetCaptureOuputPinStreamCaps -> QueryInterface(IID_IAMStreamConfig) failed, HRESULT: "
			<< res << std::endl;
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 回调中的状态（默认为未找到对应的流能力信息）
	auto errCode = StatusCode::STATUS_CODE_ERR_NOMATCH_STREAM_CAPS;
	// 遍历设备所有流能力
	auto code =
		BecamEnumDevice::GetDeviceStreamCaps(streamConfig, [streamConfig, frameInfo, &errCode](AM_MEDIA_TYPE* pmt) {
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
					std::cerr << "BecamEnumDevice::SetCaptureOuputPinStreamCaps -> SetFormat failed, HRESULT: " << res
							  << std::endl;
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
		std::cerr << "BecamEnumDevice::SetCaptureOuputPinStreamCaps -> GetDeviceStreamCaps failed, CODE: " << code
				  << std::endl;
		return code;
	}
	if (errCode != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "BecamEnumDevice::SetCaptureOuputPinStreamCaps -> GetDeviceStreamCaps callback failed, CODE: "
				  << errCode << std::endl;
		return errCode;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}
