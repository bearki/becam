
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
	// 声明图像最新更新时间
	double updateAt = 0;

public:
	// 构造函数
	inline SampleGrabberCallback() {
		// 初始化一个4MB大小的缓冲区
		this->bufferCap = 4 * 1024 * 1024;
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
		this->updateAt = sampleTime;
		this->bufferLen = bufferLen;
		memcpy(this->buffer, buffer, bufferLen);

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

		// 检查缓冲区是否为过期内容
		std::cout << "updateAt: " << this->updateAt << std::endl;

		// 缓冲区有内容，开辟指定大小的空间
		*size = this->bufferLen;
		*data = new uint8_t[this->bufferLen];
		// 拷贝内容
		memcpy(*data, this->buffer, this->bufferLen);

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