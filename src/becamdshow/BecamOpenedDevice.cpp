#include "BecamOpenedDevice.hpp"
#include "BecamDirectShow.hpp"
#include "BecamEnumDevice.hpp"

/**
 * @brief 构造函数
 */
BecamOpenedDevice::BecamOpenedDevice() {
	// 初始化COM库
	auto res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(res)) {
		// COM库初始化失败了
		this->comInited = false;
	} else {
		// COM库初始化成功了
		this->comInited = true;
	}
}

/**
 * @brief 析构函数
 */
BecamOpenedDevice::~BecamOpenedDevice() {
	// 释放媒体控制器
	if (this->mediaControl != nullptr) {
		this->mediaControl->StopWhenReady(); // 停止媒体捕获
		this->mediaControl->Release();
		this->mediaControl = nullptr;
	}
	// 释放空渲染器
	if (this->nullRender != nullptr) {
		this->nullRender->Release();
		this->nullRender = nullptr;
	}
	// 将图像构建器（画布）中的组件移除
	if (this->graphBuilder != nullptr) {
		// 将视频帧抓取器从图像构建器中移除
		if (this->sampleGrabber != nullptr) {
			this->graphBuilder->RemoveFilter(this->sampleGrabber);
		}
		// 将捕获筛选器从图像构建器移除
		if (this->captureFilter != nullptr) {
			this->graphBuilder->RemoveFilter(this->captureFilter);
		}
	}

	// 释放视频帧抓取器（处理接口）
	if (this->sampleGrabberIntf != nullptr) {
		this->sampleGrabberIntf->Release();
		this->sampleGrabberIntf = nullptr;
	}
	// 释放视频帧抓取器（处理）
	if (this->sampleGrabber != nullptr) {
		this->sampleGrabber->Release();
		this->sampleGrabber = nullptr;
	}
	// 释放视频帧抓取器回调（处理接口回调）
	if (this->sampleGrabberCallback != nullptr) {
		delete this->sampleGrabberCallback;
		this->sampleGrabberCallback = nullptr;
	}
	// 释放图像构建器（画布）
	if (this->graphBuilder != nullptr) {
		this->graphBuilder->Release();
		this->graphBuilder = nullptr;
	}
	// 释放捕获筛选器输出端口
	if (this->captureOuputPin != nullptr) {
		this->captureOuputPin->Release();
		this->captureOuputPin = nullptr;
	}
	// 释放捕获筛选器（相机）
	if (this->captureFilter != nullptr) {
		this->captureFilter->Release();
		this->captureFilter = nullptr;
	}
	// 是否需要释放COM库
	if (this->comInited) {
		// 释放COM库
		CoUninitialize();
		// 重置标记
		this->comInited = false;
	}
}

/**
 * @brief 打开设备
 *
 * @param devicePath [in] 设备路径
 * @param frameInfo [in] 设置的视频帧信息
 * @return 状态码
 */
StatusCode BecamOpenedDevice::Open(const std::string devicePath, const VideoFrameInfo* frameInfo) {
	// 构建设备枚举类（不要初始化COM库）
	auto enumDevice = BecamEnumDevice(false);

	// 声明设备实例
	IMoniker* moniker;
	// 筛选设备
	auto code = enumDevice.GetDeviceRef(devicePath, moniker);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 释放COM库
		CoUninitialize();
		std::cerr << "BecamOpenedDevice::Open -> GetDeviceRef failed, CODE: " << code << std::endl;
		return code;
	}

	// 绑定到捕获筛选器实例
	auto res = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&this->captureFilter);
	// 释放设备实例
	moniker->Release();
	// 检查是否绑定成功
	if (FAILED(res)) {
		// 绑定设备实例失败
		std::cerr << "BecamOpenedDevice::Open -> BindToObject failed, HRESULT: " << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_SELECTED_DEVICE;
	}

	// 获取捕获筛选器的输出端口
	this->captureOuputPin = BecamEnumDevice::GetPin(this->captureFilter, PINDIR_OUTPUT);
	if (this->captureOuputPin == nullptr) {
		// 获取PIN接口失败
		std::cerr << "BecamOpenedDevice::Open -> GetPin failed" << std::endl;
		return StatusCode::STATUS_CODE_ERR_GET_STREAM_CAPS;
	}

	// 配置设备流能力
	code = BecamEnumDevice::SetCaptureOuputPinStreamCaps(this->captureOuputPin, *frameInfo);
	if (code != StatusCode::STATUS_CODE_SUCCESS) {
		// 配置设备流能力失败
		std::cerr << "BecamOpenedDevice::Open -> SetCaptureOuputPinStreamCaps failed, CODE: " << code << std::endl;
		return code;
	}

	// ----------------------------- 创建各个组件实例 ----------------------------- //

	// 创建图像构建器（画布）
	res =
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
	res = this->graphBuilder->AddFilter(this->captureFilter, L"Video Capture Filter");
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

	// ----------------------- 连接各实例的输入、输出端口 ----------------------- //

	// 采集器输入端口
	auto dst = BecamEnumDevice::GetPin(this->sampleGrabber, PINDIR_INPUT);
	if (dst == nullptr) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}
	// 连接捕获器输出和采集器输入
	res = this->graphBuilder->Connect(this->captureOuputPin, dst);
	// 释放引用
	this->captureOuputPin->Release();
	this->captureOuputPin = nullptr;
	dst->Release();
	dst = nullptr;
	// 检查连接结果
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_CAPTURE_GRABBER;
	}

	// 采集器输出端口
	auto grabber = BecamEnumDevice::GetPin(this->sampleGrabber, PINDIR_OUTPUT);
	if (FAILED(res)) {
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_GRABBER_RENDER;
	}
	// 空渲染器输入端口
	auto render = BecamEnumDevice::GetPin(this->nullRender, PINDIR_INPUT);
	if (FAILED(res)) {
		// 释放引用
		grabber->Release();
		grabber = nullptr;
		// 连接失败
		return StatusCode::STATUS_CODE_ERR_GRABBER_RENDER;
	}
	// 连接采集和渲染
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

	// ----------------------- 获取、配置控制接口 ----------------------- //

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

	// 配置回调
	this->sampleGrabberCallback = new BecamSampleGrabberCallback();
	this->sampleGrabberIntf->SetCallback(this->sampleGrabberCallback, 1);

	// 开始消耗帧，然后就结束了
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

	// 每次取帧的状态码
	auto code = StatusCode::STATUS_CODE_SUCCESS;
	// 最多尝试50次
	for (size_t i = 0; i < 50; i++) {
		// 获取帧
		code = this->sampleGrabberCallback->GetFrame(data, size);
		if (code == StatusCode::STATUS_CODE_SUCCESS) {
			// 取到了，立即返回正常结果
			return code;
		}
		// 延迟1ms
		Sleep(1);
	}

	// 返回异常结果
	return code;
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
