#pragma once

#ifndef _BECAM_SAMPLE_GRABBER_CALLBACK_H_
#define _BECAM_SAMPLE_GRABBER_CALLBACK_H_

#include <becam/becam.h>
#include <mutex>
#include <qedit.h>

/**
 * @brief 采集器回调实现
 *
 */
class BecamSampleGrabberCallback : public ISampleGrabberCB {
private:
	// 声明互斥锁
	std::mutex mtx;
	// 声明图像缓冲区
	uint8_t* buffer = nullptr;
	// 声明图像缓冲区当前容量
	uint32_t bufferCap = 0;
	// 声明图像缓冲区当前使用量
	uint32_t bufferLen = 0;
	// 图像是否有更新
	bool bufferUpdated = false;

public:
	/**
	 * @brief 构造函数
	 */
	BecamSampleGrabberCallback();

	/**
	 * @brief 析构函数
	 */
	~BecamSampleGrabberCallback();

	// 引用计数
	inline ULONG STDMETHODCALLTYPE AddRef() { return 1; }
	// 引用计数
	inline ULONG STDMETHODCALLTYPE Release() { return 1; }
	// 查询接口
	inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) {
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void*)static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	// 采集样品回调
	inline HRESULT SampleCB(double sampleTime, IMediaSample* sample) { return S_OK; };
	// 采集缓冲区回调
	HRESULT BufferCB(double sampleTime, BYTE* buffer, LONG bufferLen);

	/**
	 * @brief 获取视频帧
	 *
	 * @param data 视频帧流
	 * @param size 视频帧流大小
	 * @return 状态码
	 */
	StatusCode GetFrame(uint8_t** data, size_t* size);

	/**
	 * @brief 释放视频帧
	 *
	 * @param data 视频帧流
	 */
	void FreeFrame(uint8_t** data);
};

#endif
