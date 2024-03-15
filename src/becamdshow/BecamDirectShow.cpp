#include "BecamDirectShow.hpp"
#include "DShowAmMediaType.hpp"
#include "MonikerPropReader.hpp"
#include "StringConvert.hpp"

#include <vector>

/**
 * @brief 枚举设备列表
 *
 * @param callback [in] 回调函数（回调完成IMoniker将立即释放，回调返回false将立即停止枚举）
 * @return 状态码
 */
StatusCode BecamDirectShow::enumDevices(std::function<CbRes(IMoniker*)> callback) {
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
		auto cbRes = callback(pMoniker);
		if (cbRes == CbRes::FREE_AND_NOBREAK) {
			// 释放设备实例
			pMoniker->Release();
			// 继续枚举
			continue;
		} else if (CbRes::FREE_AND_BREAK) {
			// 释放设备实例
			pMoniker->Release();
			// 终止枚举
			break;
		} else if (cbRes == CbRes::NOFREE_AND_NOBREAK) {
			// 不释放AM_MEDIA_TYPE
			// 继续枚举
			continue;
		} else if (cbRes == CbRes::NOFREE_AND_BREAK) {
			// 不释放AM_MEDIA_TYPE
			// 终止枚举
			break;
		} else {
			// 释放设备实例
			pMoniker->Release();
			// 继续枚举
			continue;
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

/**
 * @brief 枚举设备支持的流能力
 *
 * @param pMoniker [in] 设备实例
 * @param callback [in] 回调函数（将依据回调结果处理资源及剩余枚举）
 * @return 状态码
 */
StatusCode BecamDirectShow::enumStreamCaps(IMoniker* pMoniker, std::function<CbRes(AM_MEDIA_TYPE*)> callback) {
	// 检查参数
	if (pMoniker == nullptr || callback == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 绑定到IBaseFilter接口
	// TODO: 目前BindToObject会发生内存泄漏，没搞懂咋回事，有空再弄吧
	IBaseFilter* pFilter = nullptr;
	auto res = pMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pFilter);
	if (FAILED(res)) {
		// 绑定接口失败了
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 获取PIN接口
	auto pin = this->getPin(pFilter, PINDIR_OUTPUT);
	if (pin == nullptr) {
		// 释放设备接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取PIN接口失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 获取流能力接口
	IAMStreamConfig* pStreamConfig = nullptr;
	res = pin->QueryInterface(IID_IAMStreamConfig, (void**)&pStreamConfig);
	if (FAILED(res)) {
		// 释放PIN接口
		pin->Release();
		pin = nullptr;
		// 释放设备接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取流能力接口失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 声明用来接收支持的流能力总数量
	int count, size;
	// 获取流能力总数量
	res = pStreamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(res)) {
		// 释放流能力接口
		pStreamConfig->Release();
		pStreamConfig = nullptr;
		// 释放PIN接口
		pin->Release();
		pin = nullptr;
		// 释放设备接口
		pFilter->Release();
		pFilter = nullptr;
		// 获取流能力总数量失败
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

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
		// 检查一下
		if (pmt->majortype != MEDIATYPE_Video || pmt->formattype != FORMAT_VideoInfo || pmt->pbFormat == nullptr) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			// 信息不是想要的
			continue;
		}
		// 执行回调，并检查回调结果
		auto cbRes = callback(pmt);
		if (cbRes == CbRes::FREE_AND_NOBREAK) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			// 继续枚举
			continue;
		} else if (CbRes::FREE_AND_BREAK) {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			// 终止枚举
			break;
		} else if (cbRes == CbRes::NOFREE_AND_NOBREAK) {
			// 不释放AM_MEDIA_TYPE
			// 继续枚举
			continue;
		} else if (cbRes == CbRes::NOFREE_AND_BREAK) {
			// 不释放AM_MEDIA_TYPE
			// 终止枚举
			break;
		} else {
			// 释放AM_MEDIA_TYPE
			_DeleteMediaType(pmt);
			// 继续枚举
			continue;
		}
	}

	// 释放流能力接口
	pStreamConfig->Release();
	pStreamConfig = nullptr;
	// 释放PIN接口
	pin->Release();
	pin = nullptr;
	// 释放设备接口
	pFilter->Release();
	pFilter = nullptr;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备友好名称
 *
 * @param pMoniker [in] 设备实例
 * @param devicePath [out] 设备友好名称
 * @return 状态码
 */
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

/**
 * @brief 获取设备路径
 *
 * @param pMoniker [in] 设备实例
 * @param devicePath [out] 设备路径
 * @return 状态码
 */
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

/**
 * @brief 获取设备支持的流能力
 *
 * @param pMoniker [in] 设备实例
 * @param reply [out] 设备流能力列表
 * @param replySize [out] 设备流能力列表大小
 * @return 状态码
 */
StatusCode BecamDirectShow::getMonikerStreamCaps(IMoniker* pMoniker, VideoFrameInfo** reply, size_t* replySize) {
	// 检查参数
	if (pMoniker == nullptr || reply == nullptr || replySize == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 置零
	*reply = nullptr;
	*replySize = 0;
	// 声明vector
	std::vector<VideoFrameInfo> replyVec;

	// 执行流能力枚举
	auto code = this->enumStreamCaps(pMoniker, [&replyVec](AM_MEDIA_TYPE* pmt) {
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
		return CbRes::FREE_AND_NOBREAK;
	});
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
 * @brief 筛选设备支持的流能力
 *
 * @param pMoniker [in] 设备实例
 * @param input [in] 视频帧信息
 * @param reply [out] 设备流能力资源实例（外部请使用reply->Release()释放资源）
 * @return 状态码
 */
StatusCode BecamDirectShow::filterMonikerStreamCaps(IMoniker* pMoniker, const VideoFrameInfo* input,
													AM_MEDIA_TYPE** reply) {
	// 检查参数
	if (pMoniker == nullptr || input == nullptr || reply == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INTERNAL_PARAM;
	}

	// 置零
	reply = nullptr;
	// 执行流能力枚举
	auto code = this->enumStreamCaps(pMoniker, [input, &reply](AM_MEDIA_TYPE* pmt) {
		// 提取信息
		VIDEOINFOHEADER* videoInfoHdr = (VIDEOINFOHEADER*)pmt->pbFormat;
		auto width = videoInfoHdr->bmiHeader.biWidth;		 // 提取宽度
		auto height = videoInfoHdr->bmiHeader.biHeight;		 // 提取高度
		auto fps = 10000000 / videoInfoHdr->AvgTimePerFrame; // 提取帧率
		auto format = videoInfoHdr->bmiHeader.biCompression; // 提取格式
		// 信息是否一致
		if (input->width == width && input->height == height && input->fps == fps && input->format == format) {
			// 信息一致,保留并返回资源
			*reply = pmt;
			// 保留资源，终止枚举
			return CbRes::NOFREE_AND_BREAK;
		}
		// 继续查找下一个
		return CbRes::FREE_AND_NOBREAK;
	});
	// 检查结果
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 失败了
		return code;
	}

	// 检查资源是否有枚举到
	if (reply == nullptr) {
		// 没有匹配到流能力
		return StatusCode::STATUS_CODE_ERR_NOMATCH_STREAM_CAPS;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取设备列表
 *
 * @param reply [out] 响应参数
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
			return CbRes::FREE_AND_NOBREAK;
		}

		// 获取设备路径
		std::string devicePath = "";
		code = this->getMonikerDevicePath(pMoniker, devicePath);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return CbRes::FREE_AND_NOBREAK;
		}

		// 获取设备位置信息
		std::string locationInfo = "";

		// 获取设备流能力
		VideoFrameInfo* list = nullptr;
		size_t listSize = 0;
		code = this->getMonikerStreamCaps(pMoniker, &list, &listSize);
		if (code != StatusCode::STATUS_CODE_SUCCESS) {
			// 继续枚举
			return CbRes::FREE_AND_NOBREAK;
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
		return CbRes::FREE_AND_NOBREAK;
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
 * @param devicePath [in] 设备路径
 * @param frameInfo [in] 设置的视频帧信息
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
	// 声明设备流能力实例
	AM_MEDIA_TYPE* mt = nullptr;
	// 预声明绑定结果
	HRESULT res = S_OK;
	// 回调中发生的错误码
	auto callbackCode = StatusCode::STATUS_CODE_SUCCESS;
	// 枚举设备，直到找到合适的设备
	auto code =
		this->enumDevices([this, devicePath, frameInfo, &pCaptureFilter, &mt, &res, &callbackCode](IMoniker* pMoniker) {
			// 获取设备路径
			std::string tmpDevicePath = "";
			callbackCode = this->getMonikerDevicePath(pMoniker, tmpDevicePath);
			if (callbackCode != StatusCode::STATUS_CODE_SUCCESS) {
				// 失败了
				return CbRes::FREE_AND_BREAK;
			}
			// 是否是这个设备
			if (tmpDevicePath != devicePath) {
				// 不是这个设备，继续查
				return CbRes::FREE_AND_NOBREAK;
			}

			// 筛选设备流能力
			callbackCode = this->filterMonikerStreamCaps(pMoniker, frameInfo, &mt);
			if (callbackCode != StatusCode::STATUS_CODE_SUCCESS) {
				// 失败了
				return CbRes::FREE_AND_BREAK;
			}

			// 绑定到捕获筛选器
			res = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pCaptureFilter);
			if (FAILED(res)) {
				// 释放流能力
				_DeleteMediaType(mt);
				mt = nullptr;
				// 绑定设备实例事变
				callbackCode = StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
				// 失败了
				return CbRes::FREE_AND_BREAK;
			}

			// 找到后立即结束枚举
			return CbRes::FREE_AND_BREAK;
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
	this->openedDevice = new BecamOpenedDevice(pCaptureFilter, mt);
	code = this->openedDevice->Open();
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 打开失败，关闭实例
		delete this->openedDevice;
		this->openedDevice = nullptr;
		return code;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}
