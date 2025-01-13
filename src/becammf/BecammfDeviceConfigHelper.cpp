#include "BecammfDeviceConfigHelper.hpp"
#include <iostream>
#include <vector>

/**
 * @implements 实现构造函数
 */
BecammfDeviceConfigHelper::BecammfDeviceConfigHelper(IMFMediaSource* pSource) {
	// 赋值设备源
	this->pSource = pSource;
}

/**
 * @implements 实现析构函数
 */
BecammfDeviceConfigHelper::~BecammfDeviceConfigHelper() {
	// 释放资源
	this->ReleaseCurrent();
}

/**
 * @implements 实现释放当前资源
 */
void BecammfDeviceConfigHelper::ReleaseCurrent() {
	// 释放媒体类型处理器
	SafeRelease(&this->pHandler);
	// 释放视频流的流描述符
	SafeRelease(&this->pSD);
	// 释放演示文稿描述符
	SafeRelease(&this->pPD);
}

/**
 * @implements 实现转换视频帧格式的类型：FOURCC -> GUID
 */
GUID BecammfDeviceConfigHelper::FourccToGuid(FOURCC fmt) {
	// https://learn.microsoft.com/zh-cn/windows/win32/medfound/video-subtype-guids#creating-subtype-guids-from-fourccs-and-d3dformat-values
	GUID tmp = MFVideoFormat_Base;
	tmp.Data1 = fmt;
	return tmp;
}

/**
 * @implements 实现转换视频帧格式的类型：GUID -> FOURCC
 */
FOURCC BecammfDeviceConfigHelper::GuidToFourcc(GUID fmt) {
	// https://learn.microsoft.com/zh-cn/windows/win32/medfound/video-subtype-guids#creating-subtype-guids-from-fourccs-and-d3dformat-values
	return fmt.Data1;
}

/**
 * @implements 实现获取设备支持的配置列表
 */
StatusCode BecammfDeviceConfigHelper::GetDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize) {
	// 释放上一次的资源，防止内存重复开辟
	this->ReleaseCurrent();

	// 获取演示文稿描述符
	auto res = this->pSource->CreatePresentationDescriptor(&this->pPD);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> this->pSource->CreatePresentationDescriptor(&this->pPD)() failed, "
					 "HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_CREATE_PRESENT_DESC;
	}

	// 获取视频流的流描述符
	BOOL fSelected = false;
	res = this->pPD->GetStreamDescriptorByIndex(0, &fSelected, &this->pSD);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> this->pPD->GetStreamDescriptorByIndex(0, &fSelected, &this->pSD) "
					 "failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_GET_STREAM_DESC;
	}

	// 获取媒体类型处理器
	res = this->pSD->GetMediaTypeHandler(&this->pHandler);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> this->pSD->GetMediaTypeHandler(&this->pHandler) failed, HRESULT: "
				  << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_GET_MEDIA_TYPE_HANDLER;
	}

	// 获取支持的媒体类型数量
	DWORD cTypes = 0;
	res = this->pHandler->GetMediaTypeCount(&cTypes);
	if (FAILED(res)) {
		std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> this->pHandler->GetMediaTypeCount(&cTypes) failed, HRESULT: " << res
				  << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_GET_MEDIA_TYPE_COUNT;
	}

	// 临时声明响应列表
	std::vector<VideoFrameInfo> frameList;
	// 遍历支持的媒体资源类型
	IMFMediaType* pType = nullptr;
	for (DWORD i = 0; i < cTypes; i++) {
		// 通过索引获取媒体资源类型
		res = this->pHandler->GetMediaTypeByIndex(i, &pType);
		if (FAILED(res)) {
			std::cerr
				<< "BecammfDeviceConfigHelper::GetDeviceConfigList -> this->pHandler->GetMediaTypeByIndex(i, &pType) failed, HRESULT: "
				<< res << std::endl;
			return StatusCode::STATUS_CODE_MF_ERR_GET_MEDIA_TYPE;
		}

		// 获取帧格式
		GUID subtype;
		res = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
		if (FAILED(res)) {
			// 释放媒体资源类型
			SafeRelease(&pType);
			std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> pType->GetGUID(MF_MT_SUBTYPE, &subtype) "
						 "failed, HRESULT: "
					  << res << std::endl;
			// 忽略，继续下一个
			continue;
		}
		auto frameFormat = BecammfDeviceConfigHelper::GuidToFourcc(subtype);

		// 获取帧大小
		UINT32 width = 0;
		UINT32 height = 0;
		res = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
		if (FAILED(res)) {
			// 释放媒体资源类型
			SafeRelease(&pType);
			std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> MFGetAttributeRatio(pType, MF_MT_FRAME_SIZE) "
						 "failed, HRESULT: "
					  << res << std::endl;
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
			std::cerr << "BecammfDeviceConfigHelper::GetDeviceConfigList -> MFGetAttributeRatio(pType, MF_MT_FRAME_RATE) failed, HRESULT: "
					  << res << std::endl;
			// 忽略，继续下一个
			continue;
		}

		// 添加到列表中
		frameList.push_back(VideoFrameInfo{
			format : frameFormat,
			width : width,
			height : height,
			fps : numerator / denominator,
		});

		// 释放媒体资源类型
		SafeRelease(&pType);
	}

	// 是否需要赋值列表
	if (frameList.size() > 0) {
		replySize = frameList.size();
		reply = new VideoFrameInfo[replySize];
		memcpy(reply, frameList.data(), replySize * sizeof(VideoFrameInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放已获取的设备支持的配置列表
 */
void BecammfDeviceConfigHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 检查
	if (input == nullptr || inputSize == 0) {
		return;
	}
	// 释放
	delete[] input;
	input = nullptr;
	inputSize = 0;
}
