#pragma once

#ifndef _BECAM_SAFE_RELEASE_H_
#define _BECAM_SAFE_RELEASE_H_

/**
 * @brief 智能指针安全释放助手函数
 *
 * @param ppt 智能指针储存变量的地址
 */
template <class T> static void SafeRelease(T** ppT) {
	if (*ppT) {
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

#endif
