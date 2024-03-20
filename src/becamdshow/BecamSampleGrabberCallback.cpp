#include "BecamSampleGrabberCallback.hpp"

/**
 * @brief 构造函数
 */
BecamSampleGrabberCallback::BecamSampleGrabberCallback() {
	// 初始化一个1MB大小的缓冲区
	this->bufferCap = 1 * 1024 * 1024;
	this->bufferLen = 0;
	this->buffer = new uint8_t[this->bufferCap];
}

/**
 * @brief 析构函数
 */
BecamSampleGrabberCallback::~BecamSampleGrabberCallback() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);
	// 释放缓冲区
	this->bufferCap = 0;
	this->bufferLen = 0;
	if (this->buffer != nullptr) {
		delete[] this->buffer;
		this->buffer = nullptr;
	}
	this->bufferUpdated = false;
}

// 采集样品回调
STDMETHODIMP BecamSampleGrabberCallback::SampleCB(double sampleTime, IMediaSample* sample) { return S_OK; };

/**
 * @brief 采集器缓冲区回调
 */
STDMETHODIMP BecamSampleGrabberCallback::BufferCB(double sampleTime, BYTE* buffer, LONG bufferLen) {
	// 视频帧是否有效
	if (buffer == nullptr || bufferLen == 0) {
		return S_OK;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);
	// 检查缓冲区容量是否充足
	if (bufferLen > this->bufferCap) {
		// 容量不足，释放原空间，
		delete[] this->buffer;
		// 重开空间
		this->bufferCap = bufferLen + 1024;
		this->buffer = new uint8_t[this->bufferCap];
	}

	// 拷贝内容
	this->bufferLen = bufferLen;
	memcpy(this->buffer, buffer, bufferLen);
	// 将缓冲区标记为已更新
	this->bufferUpdated = true;

	// OK
	return S_OK;
}

/**
 * @brief 获取视频帧
 *
 * @param data 视频帧流
 * @param size 视频帧流大小
 * @return 状态码
 */
StatusCode BecamSampleGrabberCallback::GetFrame(uint8_t** data, size_t* size) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);
	// 检查缓冲区是否有内容
	if (this->bufferLen <= 0) {
		// 无内容
		return StatusCode::STATUS_CODE_ERR_FRAME_EMPTY;
	}

	// 看下缓冲区是否有更新
	if (!this->bufferUpdated) {
		return StatusCode::STATUS_CODE_ERR_FRAME_NOT_UPDATE;
	}

	// 缓冲区有内容，开辟指定大小的空间
	*size = this->bufferLen;
	*data = new uint8_t[this->bufferLen];
	// 拷贝内容
	memcpy(*data, this->buffer, this->bufferLen);
	// 将缓冲区标记为未更新
	this->bufferUpdated = false;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @brief 释放视频帧
 *
 * @param data 视频帧流
 */
void BecamSampleGrabberCallback::FreeFrame(uint8_t** data) {
	// 检查参数
	if (data == nullptr || *data == nullptr) {
		// 忽略
		return;
	}

	// 释放内存
	delete[] *data;
	// 释放指针
	*data = nullptr;
}
