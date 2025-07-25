#include <combaseapi.h>

// 自动COM类封装
class AutoCOM {
public:
	AutoCOM() : initialized_(false) {
		HRESULT hr = CoGetApartmentType(&aptType_, &qualifier_);
		if (hr == CO_E_NOTINITIALIZED) {
			hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr))
				initialized_ = true;
		}
	}

	~AutoCOM() {
		if (initialized_)
			CoUninitialize();
	}

private:
	APTTYPE aptType_;
	APTTYPEQUALIFIER qualifier_;
	bool initialized_;
};