#ifndef _BECAMDSHOW_MONIKER_PROP_READER_H_
#define _BECAMDSHOW_MONIKER_PROP_READER_H_

#include <becam/becam.h>
#include <strmif.h>
#include <utility>

/**
 * @brief 设备实例属性读取器
 *
 */
class MonikerPropReader {
private:
	// 属性名
	LPCOLESTR propName;
	// 属性包实例
	IPropertyBag* propBag;
	// UTF16属性值
	VARIANT friendNameUTF16;

public:
	/**
	 * @brief 构造函数
	 *
	 * @param propName 属性名
	 */
	MonikerPropReader(LPCOLESTR propName);

	/**
	 * @brief 析构函数
	 */
	~MonikerPropReader();

	/**
	 * @brief 读取属性
	 * @param pMoniker 设备实例
	 * @return 状态码和UTF16属性值引用（外部无需管理，脱离MonikerPropReader作用域会自动释放）
	 */
	std::pair<StatusCode, VARIANT> read(IMoniker* pMoniker);
};

#endif
