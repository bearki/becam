#include "becamdshow.h"

#include <dshow.h>
#include <setupapi.h>
#include <windows.h>

#include <iostream>

#include "moniker_prop_reader.cpp"
#include "string_convert.cpp"

/**
 * @brief 获取设备属性友好名称
 *
 * @param   pMoniker    设备实例
 * @return  设备友好名称（请使用`delete`释放）
 */
char *getMonikerPropWithFriendlyName(IMoniker *pMoniker)
{
    // 构建阅读器并读取属性值
    auto reader = MonikerPropReader(L"FriendlyName");
    auto res = reader.read(pMoniker);
    if (res.first != StatusCode::STATUS_CODE_SUCCESS)
    {
        // 失败返回空字符串
        return nullptr;
    }

    // UTF16转换为UTF8
    // 一般可以加一下第二个参数，顺便切换编码
    auto friendlyNameUTF8 = WcharToChar(res.second.bstrVal, CP_UTF8);
    // 转换为C字符串
    char *friendName = new char[friendlyNameUTF8.size() + 1];
    memcpy(friendName, friendlyNameUTF8.c_str(), friendlyNameUTF8.size() + 1);

    // OK
    return friendName;
}

/**
 * @brief 获取设备属性设备路径
 *
 * @param   pMoniker    设备实例
 * @return  设备路径信息
 */
char *getMonikerPropWithDevicePath(IMoniker *pMoniker)
{
    // 构建阅读器并读取属性值
    auto reader = MonikerPropReader(L"DevicePath");
    auto res = reader.read(pMoniker);
    if (res.first != StatusCode::STATUS_CODE_SUCCESS)
    {
        // 失败返回空字符串
        return nullptr;
    }

    // UTF16转换为UTF8
    // 一般可以加一下第二个参数，顺便切换编码
    auto friendlyName = WcharToChar(res.second.bstrVal, CP_UTF8);
    // 转换为C字符串
    char *ret = new char[friendlyName.size() + 1];
    memcpy(ret, friendlyName.c_str(), friendlyName.size() + 1);

    // OK
    return ret;
}

/**
 * @brief 获取设备属性位置信息
 *
 * @param   pMoniker    设备实例
 * @return  设备位置信息
 */
char *getMonikerPropWithLocationInfo(IMoniker *pMoniker)
{
    // 构建阅读器并读取属性值
    auto reader = MonikerPropReader(L"DevicePath");
    auto res = reader.read(pMoniker);
    if (res.first != StatusCode::STATUS_CODE_SUCCESS)
    {
        // 失败返回空字符串
        return nullptr;
    }

    // UTF16转换为UTF8
    // 一般可以加一下第二个参数，顺便切换编码
    auto friendlyName = WcharToChar(res.second.bstrVal, CP_UTF8);
    // 转换为C字符串
    char *ret = new char[friendlyName.size() + 1];
    memcpy(ret, friendlyName.c_str(), friendlyName.size() + 1);

    // OK
    return ret;
}

/**
 * @brief 获取设备支持的流配置
 *
 * @param   pMoniker    设备实例
 * @return  设备支持的流配置
 */
char *getMonikerWithStreamConfig(IMoniker *pMoniker)
{
    // 绑定到IBaseFilter接口
    IBaseFilter *pFilter = nullptr;
    auto hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pFilter);
    if (FAILED(hr))
    {
        // 绑定接口失败了
        return nullptr;
    }

    // 获取流配置接口
    IAMStreamConfig *pStreamConfig = nullptr;
    hr = pFilter->QueryInterface(IID_IAMStreamConfig, (void **)&pStreamConfig);
    if (FAILED(hr))
    {
        // 释放IBaseFilter接口
        pFilter->Release();
        pFilter = nullptr;
        // 获取流配置接口失败
        return nullptr;
    }

    // 声明用来接收支持的流配置总数量
    int count;
    // 获取配置总数量
    hr = pStreamConfig->GetNumberOfCapabilities(&count, NULL);
    if (FAILED(hr))
    {
        // 释放流配置接口
        pStreamConfig->Release();
        pStreamConfig = nullptr;
        // 释放IBaseFilter接口
        pFilter->Release();
        pFilter = nullptr;
        // 获取配置总数量失败
        return nullptr;
    }

    // 遍历所有流配置
    for (int i = 0; i < count; ++i)
    {
        AM_MEDIA_TYPE *pmt = NULL;
        VIDEO_STREAM_CONFIG_CAPS scc;
        hr = pStreamConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE *>(&scc));
        if (SUCCEEDED(hr))
        {
            // 分析 pmt 结构体中的信息，比如：
            // pmt->formattype 识别格式类型（如VIDEOINFOHEADER2）
            // pmt->pbFormat 指向包含分辨率等信息的具体结构体
            // ...
            DeleteMediaType(pmt);
        }
    }

    // 释放流配置接口
    pStreamConfig->Release();
    pStreamConfig = nullptr;
    // 释放IBaseFilter接口
    pFilter->Release();
    pFilter = nullptr;

    // OK
    return nullptr;
}

/**
 * @brief 获取设备列表
 *
 * @param   reply   响应参数
 * @return  状态码
 */
StatusCode GetDeviceList(GetDeviceListReply *reply)
{
    // 初始化COM库
    auto res = CoInitialize(nullptr);
    if (FAILED(res))
    {
        // 初始化COM库失败
        return StatusCode::STATUS_CODE_ERR_INIT_COM;
    }

    // 声明设备枚举器实例
    ICreateDevEnum *pDevEnum = nullptr;
    // 创建设备枚举器
    res = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pDevEnum);
    if (FAILED(res))
    {
        // 释放COM库
        CoUninitialize();
        // 创建设备枚举器失败
        return StatusCode::STATUS_CODE_ERR_CREATE_DEVICE_ENUMERATOR;
    }

    // 声明枚举接收器实例
    IEnumMoniker *pEnum = nullptr;
    // 枚举视频捕获设备
    res = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    // 错误码是否为空设备
    if (res == S_FALSE)
    {
        // 设备数量为空
        reply->deviceInfoListSize = 0;
        reply->deviceInfoList = nullptr;
        // 释放设备枚举器
        pDevEnum->Release();
        pDevEnum = nullptr;
        // 释放COM库
        CoUninitialize();
        // OK
        return StatusCode::STATUS_CODE_SUCCESS;
    }
    // 其他错误码
    if (FAILED(res))
    {
        // 置空
        reply->deviceInfoListSize = 0;
        reply->deviceInfoList = nullptr;
        // 释放设备枚举器
        pDevEnum->Release();
        pDevEnum = nullptr;
        // 释放COM库
        CoUninitialize();
        // 设备枚举失败
        return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM;
    }

    // 设备实例
    IMoniker *pMoniker = nullptr;
    // 是否查询到设备
    ULONG cFetched;
    // 遍历枚举所有设备
    while (pEnum->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        // 获取设备友好名称
        auto friendlyName = getMonikerPropWithFriendlyName(pMoniker);
        // 获取设备位置信息
        auto devicePath = getMonikerPropWithDevicePath(pMoniker);
        // 获取设备视频帧信息
        // 打印一下
        std::cout << friendlyName << "     |     " << devicePath << std::endl;
        // 释放信息
        delete friendlyName;
        friendlyName = nullptr;
        delete devicePath;
        devicePath = nullptr;
        // 释放当前设备实例
        pMoniker->Release();
        pMoniker = nullptr;
    }

    // 释放设备枚举接收器实例
    pEnum->Release();
    pEnum = nullptr;
    // 释放设备枚举器实例
    pDevEnum->Release();
    pDevEnum = nullptr;
    // 释放COM库
    CoUninitialize();

    // OK
    return StatusCode::STATUS_CODE_SUCCESS;
}