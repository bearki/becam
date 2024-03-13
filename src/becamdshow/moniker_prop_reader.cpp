#include <strmif.h>

#include <utility>

#include "becamdshow.h"

/**
 * @brief 设备实例属性读取器
 *
 */
class MonikerPropReader
{
public:
    /**
     * @brief 构造函数
     *
     * @param propName 属性名
     */
    MonikerPropReader(LPCOLESTR propName)
    {
        // 储存属性名
        this->propName = propName;
        // 初始化UTF16属性值变量空间
        VariantInit(&this->friendNameUTF16);
    };

    /**
     * @brief 析构函数
     */
    ~MonikerPropReader()
    {
        // 属性包是否需要释放
        if (this->propBag != nullptr)
        {
            this->propBag->Release();
        }
        // 释放UTF16属性值变量空间
        VariantClear(&friendNameUTF16);
    };

    /**
     * @brief 读取属性
     * @param pMoniker 设备实例
     * @return 状态码和UTF16属性值引用（外部无需管理，脱离MonikerPropReader作用域会自动释放）
     */
    std::pair<StatusCode, VARIANT> read(IMoniker *pMoniker)
    {
        // 绑定属性包
        auto res = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void **)&this->propBag);
        if (FAILED(res))
        {
            // 忽略失败的，返回空名称
            return std::make_pair(StatusCode::STATUS_CODE_ERR_GET_DEVICE_PROP, this->friendNameUTF16);
        }

        // 读取属性值
        res = this->propBag->Read(this->propName, &this->friendNameUTF16, nullptr);
        if (FAILED(res))
        {
            // 忽略失败的，返回空名称
            return std::make_pair(StatusCode::STATUS_CODE_ERR_GET_DEVICE_PROP, this->friendNameUTF16);
        }

        // 返回结果
        return std::make_pair(StatusCode::STATUS_CODE_SUCCESS, this->friendNameUTF16);
    };

private:
    // 属性名
    LPCOLESTR propName;
    // 属性包实例
    IPropertyBag *propBag;
    // UTF16属性值
    VARIANT friendNameUTF16;
};