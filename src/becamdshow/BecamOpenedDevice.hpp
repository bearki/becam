#ifndef _BECAMDSHOW_OPENED_DEVICE_H_
#define _BECAMDSHOW_OPENED_DEVICE_H_

#include "DShowAmMediaType.hpp"
#include "SampleGrabberCallback.hpp"
#include <becam/becam.h>
#include <dshow.h>
#include <qedit.h>
#include <strmif.h>

EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;

/**
 * @brief 已打开的设备实例
 *
 */
class BecamOpenedDevice {
private:
	// 捕获筛选器（选中的设备）
	IBaseFilter* pCaptureFilter = nullptr;
	// 当前使用的设备流能力
	AM_MEDIA_TYPE* mt = nullptr;
	// 图像构建器（画布）
	IGraphBuilder* pGraphBuilder = nullptr;
	// 样品采集器（采集器）
	IBaseFilter* pSampleGrabber = nullptr;
	// 样品采集器接口
	ISampleGrabber* pSampleGrabberIntf = nullptr;
	// 媒体控制器（开关控制）
	IMediaControl* pMediaControl = nullptr;
	// 空渲染器
	IBaseFilter* nullRender;
	// 样品采集器回调
	SampleGrabberCallback* sampleGrabberCallback = nullptr;

	// 枚举针脚
	IPin* getPin(IBaseFilter* pFilter, PIN_DIRECTION dir) {
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

public:
	/**
	 * @brief 构造函数
	 *
	 * @param pCaptureFilter 捕获筛选器
	 */
	BecamOpenedDevice(IBaseFilter* pCaptureFilter, AM_MEDIA_TYPE* mt) {
		this->pCaptureFilter = pCaptureFilter;
		this->mt = mt;
	};

	/**
	 * @brief 析构函数
	 */
	~BecamOpenedDevice() {
		// 释放媒体控制器
		if (this->pMediaControl != nullptr) {
			this->pMediaControl->StopWhenReady(); // 停止媒体捕获
			this->pMediaControl->Release();
			this->pMediaControl = nullptr;
		}
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
		// 释放设备流能力
		if (this->mt != nullptr) {
			_DeleteMediaType(this->mt);
			this->mt = nullptr;
		}
	};

	/**
	 * @brief 打开设备
	 *
	 * @return 状态码
	 */
	StatusCode BecamOpenedDevice::Open();
};

#endif