#include "BecamOpenedDevice.hpp"

/**
 * @brief 打开设备
 *
 * @param pCaptureOuputPin 捕获器输出端口
 * @return 状态码
 */
StatusCode BecamOpenedDevice::Open(IPin* pCaptureOuputPin) {
	// 检查捕获器是否已绑定
	if (this->pCaptureFilter == nullptr) {
		return StatusCode::STATUS_CODE_NOT_FOUND_DEVICE;
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
	if (FAILED(res)) {
		// 获取样品采集器接口失败
		return StatusCode::STATUS_CODE_ERR_GET_SAMPLE_GRABBER_INFC;
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

	// 采集器输入端口
	auto dst = this->getPin(this->pSampleGrabber, PINDIR_INPUT);
	if (dst == nullptr) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}
	// 连接捕获器输出和采集器输入
	res = this->pGraphBuilder->Connect(pCaptureOuputPin, dst);
	/// 释放引用
	dst->Release();
	dst = nullptr;
	// 检查连接结果
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}

	// 采集器输出
	auto grabber = getPin(this->pSampleGrabber, PINDIR_OUTPUT);
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_GRABBER_RENDER;
	}
	// 空渲染器接收
	auto render = getPin(this->nullRender, PINDIR_INPUT);
	if (FAILED(res)) {
		// 释放引用
		grabber->Release();
		grabber = nullptr;
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_GRABBER_RENDER;
	}
	// 连接采集和渲染STATUS_CODE_ERR_GRABBER_RENDER
	res = this->pGraphBuilder->Connect(grabber, render);
	// 释放引用
	grabber->Release();
	grabber = nullptr;
	render->Release();
	render = nullptr;
	// 检查连接结果
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_GRABBER_RENDER;
	}

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

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 获取视频帧
 *
 * @param data 视频帧流
 * @param size 视频帧流大小
 * @return 状态码
 */
StatusCode BecamOpenedDevice::GetFrame(uint8_t** data, size_t* size) {
	// 检查入参
	if (data == nullptr || size == nullptr) {
		// 参数错误
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 检查样品采集器回调是否赋值
	if (this->sampleGrabberCallback == nullptr) {
		// 没有回调
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_OPEN;
	}

	// 获取帧
	return this->sampleGrabberCallback->GetFrame(data, size);
}

/**
 * @brief 释放视频帧
 *
 * @param data 视频帧流
 */
void BecamOpenedDevice::FreeFrame(uint8_t** data) {
	// 检查参数
	if (data == nullptr || *data == nullptr) {
		return;
	}

	// 检查样品采集器回调是否赋值
	if (this->sampleGrabberCallback == nullptr) {
		// 没有回调
		return;
	}

	// 释放帧
	this->sampleGrabberCallback->FreeFrame(data);
}
