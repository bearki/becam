#pragma once

#include <becam/becam.h>
#include <mfidl.h>

#ifndef _BECAMMF_ATTRIBUTES_HELPER_H_
#define _BECAMMF_ATTRIBUTES_HELPER_H_

/**
 * Media Foundation 属性助手类
 */
class BecammfAttributesHelper {
private:
	// 属性存储器实例
	IMFAttributes* _Attributes = nullptr;

public:
	/**
	 * @brief 构造函数
	 */
	BecammfAttributesHelper();

	/**
	 * @brief 构造函数
	 *
	 * @param attributes [in] 需要托管的属性存储器实例
	 */
	BecammfAttributesHelper(IMFAttributes* attributes);

	/**
	 * @brief 析构函数
	 */
	~BecammfAttributesHelper();

	/**
	 * @brief 获取属性存储器实例存储地址
	 * 
	 * @return 属性存储器实例存储地址
	 */
	IMFAttributes** AttributesAddress();

	/**
	 * @brief 获取属性存储器实例存储地址
	 * 
	 * @return 属性存储器实例
	 */
	IMFAttributes* Attributes();
};

#endif
