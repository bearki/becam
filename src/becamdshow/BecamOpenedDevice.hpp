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
		// 释放媒体控制器
		if (this->pMediaControl != nullptr) {
			this->pMediaControl->StopWhenReady(); // 停止媒体捕获
			this->pMediaControl->Release();
			this->pMediaControl = nullptr;
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
	StatusCode Open() {
		// 检查捕获器是否已绑定
		if (this->pCaptureFilter == nullptr) {
			return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
		}
		// 检查设备流能力
		if (this->mt == nullptr) {
			return StatusCode::STATUS_CODE_ERR_NOMATCH_STREAM_CAPS;
		}

		// ----------------------------- 画布基础容器构建 ----------------------------- //

		// 创建图像构建器（画布）
		auto res = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
									(void**)&this->pGraphBuilder);
		if (FAILED(res)) {
			// 创建图像构建器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_GRAPH_BUILDER;
		}

		// 将捕获筛选器添加到图构建器（链接画布和相机）
		res = this->pGraphBuilder->AddFilter(this->pCaptureFilter, L"Video Capture Filter");
		if (FAILED(res)) {
			// 添加捕获过滤器失败
			return StatusCode::STATUS_CODE_ERR_ADD_CAPTURE_FILTER;
		}

		// ----------------------------- 采集格式配置 ----------------------------- //

		// 创建样品采集器
		res = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
							   (void**)&this->pSampleGrabber);
		if (FAILED(res)) {
			// 创建样品采集器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_SAMPLE_GRABBER;
		}
		// 获取样品采集器接口
		res = this->pSampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&this->pSampleGrabberIntf);
		if (FAILED(res))
			// 获取样品采集器接口失败
			return StatusCode::STATUS_CODE_ERR_GET_SAMPLE_GRABBER_INFC;

		// 设置媒体类型
		res = this->pSampleGrabberIntf->SetMediaType(this->mt);
		if (FAILED(res)) {
			// 设置媒体类型失败
			return StatusCode::STATUS_CODE_ERR_SET_MEDIA_TYPE;
		}

		// 将样品采集器添加到图像构建器
		res = this->pGraphBuilder->AddFilter(this->pSampleGrabber, L"Sample Grabber");
		if (FAILED(res)) {
			// 添加样品采集器失败
			return StatusCode::STATUS_CODE_ERR_ADD_SAMPLE_GRABBER;
		}

		// --------------------------- 控制、输入、输出--------------------------- //

		// 创建媒体控制（开关）
		res = pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&this->pMediaControl);
		if (FAILED(res)) {
			// 创建媒体控制器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_MEDIA_CONTROL;
		}

		// 创建空渲染器
		res = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC, IID_IBaseFilter, (void**)&this->nullRender);
		if (FAILED(res)) {
			// 创建空渲染器失败
			return StatusCode::STATUS_CODE_ERR_CREATE_NULL_RENDER;
		}

		// 将空渲染器添加到图像构建器
		res = this->pGraphBuilder->AddFilter(this->nullRender, L"bull");
		if (FAILED(res)) {
			// 创建空渲染器失败
			return StatusCode::STATUS_CODE_ERR_ADD_NULL_RENDER;
		}

		auto src = this->getPin(this->pCaptureFilter, PINDIR_OUTPUT); // 捕获器输出
		auto dst = this->getPin(this->pSampleGrabber, PINDIR_INPUT);  // 采集器接收
		// 连接捕获和采集
		res = this->pGraphBuilder->Connect(src, dst);
		if (FAILED(res)) {
			// 释放引用
			src->Release();
			dst->Release();
			// 连接失败
			return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
		}

		// 释放引用
		src->Release();
		dst->Release();

		auto grabber = getPin(this->pSampleGrabber, PINDIR_OUTPUT); // 采集器输出
		auto render = getPin(this->nullRender, PINDIR_INPUT);		// 空渲染器接收
		// 连接采集和渲染STATUS_CODE_ERR_GRABBER_RENDER
		res = this->pGraphBuilder->Connect(grabber, render);
		if (FAILED(res)) {
			// 释放引用
			grabber->Release();
			render->Release();
			// 连接失败
			return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
		}

		// 释放引用
		grabber->Release();
		render->Release();

		// 一堆东西都可以释放掉了
		this->nullRender->Release();
		this->nullRender = nullptr;
		this->pCaptureFilter->Release();
		this->pCaptureFilter = nullptr;
		this->pSampleGrabber->Release();
		this->pSampleGrabber = nullptr;
		this->pGraphBuilder->Release();
		this->pGraphBuilder = nullptr;

		// 配置回调，然后就结束了
		this->sampleGrabberCallback = new SampleGrabberCallback();
		this->pSampleGrabberIntf->SetCallback(this->sampleGrabberCallback, 1);

		// 开始消耗帧
		this->pSampleGrabberIntf->SetBufferSamples(true);
		this->pMediaControl->Run();
	};
};

#endif