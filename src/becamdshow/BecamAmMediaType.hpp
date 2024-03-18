#ifndef _BECAM_AM_MEDIA_TYPE_H_
#define _BECAM_AM_MEDIA_TYPE_H_

#include <strmif.h>

// -------------------- 重写Dshow函数，在新开发环境上不提供了 -------------------- //

// Release the format block for a media type.
static void _FreeMediaType(AM_MEDIA_TYPE& mt) {
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

// Delete a media type structure that was allocated on the heap.
static void _DeleteMediaType(AM_MEDIA_TYPE* pmt) {
	if (pmt != NULL) {
		_FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
		pmt = nullptr;
	}
}

#endif
