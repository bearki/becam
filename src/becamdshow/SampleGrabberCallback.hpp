
#ifndef _BECAMDSHOW_SAMPLE_GRABBER_CB_H_
#define _BECAMDSHOW_SAMPLE_GRABBER_CB_H_

#include <becam/becam.h>
#include <dshow.h>
#include <iostream>
#include <mutex>
#include <qedit.h>
#include <vector>

class SampleGrabberCallback : public ISampleGrabberCB {
private:
	// 声明互斥锁
	std::mutex mtx;
	// 声明图像缓冲区
	uint8_t* buffer;
	// 声明图像缓冲区当前容量
	uint32_t bufferCap = 0;
	// 声明图像缓冲区当前使用量
	uint32_t bufferLen = 0;
	// 图像是否有更新
	bool bufferUpdated = false;

public:
	// 构造函数
	inline SampleGrabberCallback() {
		// 初始化一个1MB大小的缓冲区
		this->bufferCap = 1 * 1024 * 1024;
		this->bufferLen = 0;
		this->buffer = new uint8_t[this->bufferCap];
	}

	// 析构函数
	inline ~SampleGrabberCallback() {
		// 释放缓冲区
		delete[] this->buffer;
	}

	HRESULT SampleCB(double sampleTime, IMediaSample* sample) { return S_OK; };
	HRESULT BufferCB(double sampleTime, BYTE* buffer, LONG bufferLen) {
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
	};

	inline ULONG STDMETHODCALLTYPE AddRef() { return 1; }
	inline ULONG STDMETHODCALLTYPE Release() { return 1; }
	inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) {
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void*)static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}

	/**
	 * @brief 获取视频帧
	 *
	 * @param data 视频帧流
	 * @param size 视频帧流大小
	 * @return 状态码
	 */
	StatusCode GetFrame(uint8_t** data, size_t* size) {
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
	void FreeFrame(uint8_t** data) {
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
};

#endif