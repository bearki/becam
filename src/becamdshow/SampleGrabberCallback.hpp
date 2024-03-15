
#ifndef _BECAMDSHOW_SAMPLE_GRABBER_CB_H_
#define _BECAMDSHOW_SAMPLE_GRABBER_CB_H_

#include <dshow.h>
#include <qedit.h>

class SampleGrabberCallback : public ISampleGrabberCB {
private:
	// 声明读写锁
public:
	inline SampleGrabberCallback() {}

	HRESULT SampleCB(double sampleTime, IMediaSample* sample) { return S_OK; };
	HRESULT BufferCB(double sampleTime, BYTE* buffer, LONG bufferLen) { return S_OK; };

	inline ULONG STDMETHODCALLTYPE AddRef() { return 1; }
	inline ULONG STDMETHODCALLTYPE Release() { return 1; }
	inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) {
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void*)static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
};

#endif