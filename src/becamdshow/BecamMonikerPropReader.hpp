#pragma once

#ifndef _BECAM_MONIKER_PROP_READER_H_
#define _BECAM_MONIKER_PROP_READER_H_

#include <becam/becam.h>
#include <strmif.h>
#include <utility>

/**
 * @brief 设备实例属性读取器
 *
 */
class BecamMonikerPropReader {
private:
	// 属性名
	LPCOLESTR propName = nullptr;
	// 属性包实例
	IPropertyBag* propBag = nullptr;
	// UTF16属性值
	VARIANT friendNameUTF16;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param propName 属性名
	 */
	BecamMonikerPropReader(LPCOLESTR propName);

	/**
	 * @brief 析构函数
	 */
	~BecamMonikerPropReader();

	/**
	 * @brief 读取属性
	 * @param pMoniker 设备实例
	 * @return 状态码和UTF16属性值引用（外部无需管理，脱离MonikerPropReader作用域会自动释放）
	 */
	std::pair<StatusCode, VARIANT> read(IMoniker* pMoniker);
};

#endif
