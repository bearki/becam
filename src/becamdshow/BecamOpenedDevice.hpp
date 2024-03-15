#ifndef _BECAMDSHOW_OPENED_DEVICE_H_
#define _BECAMDSHOW_OPENED_DEVICE_H_

#include <becam/becam.h>
#include <dshow.h>
#include <strmif.h>

EXTERN_C const CLSID CLSID_SampleGrabber;

/**
 * @brief 已打开的设备实例
 *
 */
class BecamOpenedDevice {
private:
	// 捕获筛选器
	IBaseFilter* pCaptureFilter = nullptr;
	// 图像构建器
	IGraphBuilder* pGraphBuilder = nullptr;
	// 样品采集器
	IBaseFilter* pSampleGrabber = nullptr;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param pCaptureFilter 捕获筛选器
	 */
	BecamOpenedDevice(IBaseFilter* pCaptureFilter) { this->pCaptureFilter = pCaptureFilter; };

	/**
	 * @brief 析构函数
	 */
	~BecamOpenedDevice() {
		// 将样品采集器从图像构建器中移除
		if (this->pSampleGrabber != nullptr && this->pGraphBuilder != nullptr) {
			this->pGraphBuilder->RemoveFilter(this->pSampleGrabber);
		}
		// 释放样品采集器
		if (this->pSampleGrabber != nullptr) {
			this->pSampleGrabber->Release();
			this->pSampleGrabber = nullptr;
		}
		// 将捕获筛选器从图像构建器移除
		if (this->pCaptureFilter != nullptr && this->pGraphBuilder != nullptr) {
			this->pGraphBuilder->RemoveFilter(this->pCaptureFilter);
		}
		// 释放捕获筛选器
		if (this->pCaptureFilter != nullptr) {
			this->pCaptureFilter->Release();
			this->pCaptureFilter = nullptr;
		}
		// 释放图像构建器
		if (this->pGraphBuilder != nullptr) {
			this->pGraphBuilder->Release();
			this->pGraphBuilder = nullptr;
		}
	};

	StatusCode Open() {
		// 检查捕获器是否已绑定
		if (this->pCaptureFilter == nullptr) {
			return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
		}

		// 创建图像构建器
		auto res = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
									(void**)&this->pGraphBuilder);
		if (FAILED(res)) {
			// 创建图像构建器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_GRAPH_BUILDER;
		}

		// 将捕获筛选器添加到图构建器
		res = this->pGraphBuilder->AddFilter(this->pCaptureFilter, L"Video Capture Filter");
		if (FAILED(res)) {
			// 添加捕获过滤器失败
			return StatusCode::STATUS_CODE_ERR_ADD_CAPTURE_FILTER;
		}

		// 创建样品采集器
		res = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
							   (void**)&this->pSampleGrabber);
		if (FAILED(res)) {
			// 创建样品采集器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_SAMPLE_GRABBER;
		}

		// 将样品采集器添加到图像构建器
		res = this->pGraphBuilder->AddFilter(this->pSampleGrabber, L"Sample Grabber");
		if (FAILED(res)) {
			// 添加样品采集器失败
			return StatusCode::STATUS_CODE_ERR_ADD_SAMPLE_GRABBER;
		}
	};
};

#endif