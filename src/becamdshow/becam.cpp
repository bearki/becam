#include <becam/becam.h>
#include <dshow.h>
#include <setupapi.h>
#include <windows.h>

#include <iostream>

#include "becamdshow.cpp"

/**
 * @brief 初始化Becam接口句柄
 *
 * @return Becam接口句柄
 */
BecamHandle BecamNew() {
    // 创建Becam接口句柄
    return new BecamDirectShow();
}

/**
 * @brief 释放Becam接口句柄
 *
 * @param handle Becam接口句柄
 */
void BecamFree(BecamHandle *handle) {
    // 检查参数
    if (handle == nullptr || *handle == nullptr) {
        // 参数错误
        return;
    }

    // 获取Becam接口句柄
    auto becamHandle = static_cast<BecamDirectShow *>(*handle);
    // 释放Becam接口句柄
    delete becamHandle;

    // 释放Becam接口句柄
    *handle = nullptr;
}

/**
 * @brief 获取设备列表
 *
 * @param handle Becam接口句柄
 * @param reply 输出参数
 * @return 状态码
 */
StatusCode BecamGetDeviceList(BecamHandle handle, GetDeviceListReply *reply) {
    // 检查句柄
    if (handle == nullptr) {
        return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
    }

    // 转换句柄类型
    BecamDirectShow *becamHandle = static_cast<BecamDirectShow *>(handle);
    // 获取相机列表
    return becamHandle->GetDeviceList(reply);
}

/**
 * @brief 释放设备列表
 *
 * @param handle Becam接口句柄
 * @param input 输入参数
 */
void BecamFreeDeviceList(BecamHandle handle, GetDeviceListReply *input) {
    // 检查句柄
    if (handle == nullptr) {
        return;
    }

    // 转换句柄类型
    BecamDirectShow *becamHandle = static_cast<BecamDirectShow *>(handle);
    // 执行相机列表释放
    becamHandle->FreeDeviceList(input);
}
