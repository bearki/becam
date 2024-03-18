#include "BecamOpenedDevice.hpp"

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

/**
 * @brief 构造函数
 *
 * @param captureFilter 捕获筛选器
 */
BecamOpenedDevice::BecamOpenedDevice() {}

/**
 * @brief 析构函数
 */
BecamOpenedDevice::~BecamOpenedDevice() {
	// // 释放媒体控制器
	// if (this->pMediaControl != nullptr) {
	// 	this->pMediaControl->StopWhenReady(); // 停止媒体捕获
	// 	this->pMediaControl->Release();
	// 	this->pMediaControl = nullptr;
	// }
	// // 将样品采集器从图像构建器中移除
	// if (this->pSampleGrabber != nullptr && this->pGraphBuilder != nullptr) {
	// 	this->pGraphBuilder->RemoveFilter(this->pSampleGrabber);
	// }
	// // 释放样品采集器
	// if (this->pSampleGrabber != nullptr) {
	// 	this->pSampleGrabber->Release();
	// 	this->pSampleGrabber = nullptr;
	// }
	// // 将捕获筛选器从图像构建器移除
	// if (this->captureFilter != nullptr && this->pGraphBuilder != nullptr) {
	// 	this->pGraphBuilder->RemoveFilter(this->captureFilter);
	// }
	// // 释放捕获筛选器
	// if (this->captureFilter != nullptr) {
	// 	this->captureFilter->Release();
	// 	this->captureFilter = nullptr;
	// }
	// // 释放图像构建器
	// if (this->pGraphBuilder != nullptr) {
	// 	this->pGraphBuilder->Release();
	// 	this->pGraphBuilder = nullptr;
	// }
	// // 释放采集器回调实例
	// if (this->sampleGrabberCallback != nullptr) {
	// 	delete this->sampleGrabberCallback;
	// 	this->sampleGrabberCallback = nullptr;
	// }
}

/**
 * @brief 打开设备
 *
 * @param captureFilter 捕获过滤器（相机）
 * @param captureOuputPin 捕获筛选器输出端口
 * @return 状态码
 */
StatusCode BecamOpenedDevice::Open(IBaseFilter* captureFilter, IPin* captureOuputPin) {
	// ----------------------------- 创建各个组件实例 ----------------------------- //

	// 创建图像构建器（画布）
	auto res =
		CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&this->graphBuilder);
	if (FAILED(res)) {
		// 创建图像构建器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_GRAPH_BUILDER;
	}

	// 创建视频帧抓取器（过滤器）
	res = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
						   (void**)&this->sampleGrabber);
	if (FAILED(res)) {
		// 创建视频帧抓取器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_SAMPLE_GRABBER;
	}

	// 创建空渲染器（输出）
	res = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC, IID_IBaseFilter, (void**)&this->nullRender);
	if (FAILED(res)) {
		// 创建空渲染器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_NULL_RENDER;
	}

	// ------------------------------ 关联各组件实例 ----------------------------- //

	// 将相机捕获器添加到图像构建器
	res = this->graphBuilder->AddFilter(captureFilter, L"Video Capture Filter");
	if (FAILED(res)) {
		// 添加捕获过滤器失败
		return StatusCode::STATUS_CODE_ERR_ADD_CAPTURE_FILTER;
	}

	// 将视频帧抓取器（过滤器）添加到图像构建器
	res = this->graphBuilder->AddFilter(this->sampleGrabber, L"Sample Grabber");
	if (FAILED(res)) {
		// 添加样品采集器失败
		return StatusCode::STATUS_CODE_ERR_ADD_SAMPLE_GRABBER;
	}

	// 将空渲染器添加到图像构建器
	res = this->graphBuilder->AddFilter(this->nullRender, L"bull");
	if (FAILED(res)) {
		// 创建空渲染器失败
		return StatusCode::STATUS_CODE_ERR_ADD_NULL_RENDER;
	}

	// ----------------------- 连接各实例的输入、输出端点 ----------------------- //

	// 采集器输入端口
	auto dst = getPin(this->sampleGrabber, PINDIR_INPUT);
	if (dst == nullptr) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}
	// 连接捕获器输出和采集器输入
	res = this->graphBuilder->Connect(captureOuputPin, dst);
	/// 释放引用
	dst->Release();
	dst = nullptr;
	// 检查连接结果
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}

	// 采集器输出
	auto grabber = getPin(this->sampleGrabber, PINDIR_OUTPUT);
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
	res = this->graphBuilder->Connect(grabber, render);
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
	this->sampleGrabber->Release();
	this->sampleGrabber = nullptr;
	this->graphBuilder->Release();
	this->graphBuilder = nullptr;

	// 获取视频帧抓取器接口
	res = this->sampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&this->sampleGrabberIntf);
	if (FAILED(res)) {
		// 获取视频帧抓取器接口失败
		return StatusCode::STATUS_CODE_ERR_GET_SAMPLE_GRABBER_INFC;
	}
	// 创建媒体控制（开关）
	res = graphBuilder->QueryInterface(IID_IMediaControl, (void**)&this->mediaControl);
	if (FAILED(res)) {
		// 创建媒体控制器失败
		return StatusCode::STATUS_CODE_ERR_CREATE_MEDIA_CONTROL;
	}

	// 配置回调，然后就结束了
	this->sampleGrabberCallback = new BecamSampleGrabberCallback();
	this->sampleGrabberIntf->SetCallback(this->sampleGrabberCallback, 1);

	// 开始消耗帧
	this->sampleGrabberIntf->SetBufferSamples(true);
	this->mediaControl->Run();

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
