#pragma once

#ifndef _BECAM_DSHOW_H_
#define _BECAM_DSHOW_H_
#define BECAM_DSHOW_API __declspec(dllexport)

// 引入必要头文件
#include <stdint.h>

// StatusCode 状态码定义
enum StatusCode {
    STATUS_CODE_SUCCESS = 0,                         // 成功
    STATUS_CODE_ERR_INIT_COM = -1,                   // 初始化COM库失败
    STATUS_CODE_ERR_CREATE_DEVICE_ENUMERATOR = -2,   // 创建设备枚举器失败
    STATUS_CODE_ERR_DEVICE_ENUM = -3,                // 设备枚举失败
    STATUS_CODE_ERR_GET_DEVICE_PROP = -4,            // 获取设备属性失败
};

// VideoFrameInfo 视频帧信息
struct VideoFrameInfo {
    int32_t format;   // 格式
    int32_t width;    // 分辨率宽度
    int32_t height;   // 分辨率高度
    int32_t fps;      // 分辨率帧率
};

// DeviceInfo 设备信息
struct DeviceInfo {
    char *name;                      // 设备友好名称
    char *locationInfo;              // 设备位置信息
    size_t frameInfoListSize;        // 支持的视频帧数量
    VideoFrameInfo *frameInfoList;   // 支持的视频帧列表
};

// GetDeviceListReply 获取设备列表响应参数
struct GetDeviceListReply {
    size_t deviceInfoListSize;    // 设备数量
    DeviceInfo *deviceInfoList;   // 设备信息列表
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 获取设备列表
 *
 * @param   reply   响应参数
 * @return  StatusCode  状态码
 */
BECAM_DSHOW_API StatusCode GetDeviceList(GetDeviceListReply *reply);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BECAM_DSHOW_H_ */