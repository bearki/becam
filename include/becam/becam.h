#pragma once

#ifndef _BECAM_H_
#define _BECAM_H_
#define _BECAM_API_ __declspec(dllexport)

// 引入必要头文件
#include <stdint.h>

// Becam接口句柄
typedef void* BecamHandle;

// StatusCode 状态码定义
enum StatusCode {
	STATUS_CODE_SUCCESS,				   // 成功
	STATUS_CODE_NOT_FOUND_DEVICE,		   // 未找到设备
	STATUS_CODE_ERR_HANDLE_EMPTY,		   // Becam接口句柄未初始化
	STATUS_CODE_ERR_INPUT_PARAM,		   // 传入参数错误
	STATUS_CODE_ERR_INTERNAL_PARAM,		   // 内部参数错误
	STATUS_CODE_ERR_INIT_COM,			   // 初始化COM库失败（仅windows DirectShow实现存在该错误码）
	STATUS_CODE_ERR_CREATE_ENUMERATOR,	   // 创建设备枚举器失败
	STATUS_CODE_ERR_DEVICE_ENUM,		   // 设备枚举失败
	STATUS_CODE_ERR_GET_DEVICE_PROP,	   // 获取设备属性失败
	STATUS_CODE_ERR_GET_VIDEO_FRAME,	   // 获取视频帧信息失败
	STATUS_CODE_ERR_SELECTED_DEVICE,	   // 选择设备失败
	STATUS_CODE_ERR_CREATE_GRAPH_BUILDER,  // 创建图像构建器失败
	STATUS_CODE_ERR_ADD_CAPTURE_FILTER,	   // 添加捕获过滤器失败
	STATUS_CODE_ERR_CREATE_SAMPLE_GRABBER, // 创建样品采集器失败
	STATUS_CODE_ERR_ADD_SAMPLE_GRABBER,	   // 添加样品采集器失败
};

// VideoFrameInfo 视频帧信息
struct VideoFrameInfo {
	int32_t format; // 格式
	int32_t width;	// 分辨率宽度
	int32_t height; // 分辨率高度
	int32_t fps;	// 分辨率帧率
};

// DeviceInfo 设备信息
struct DeviceInfo {
	char* name;					   // 设备友好名称
	char* devicePath;			   // 设备路径
	char* locationInfo;			   // 设备位置信息
	size_t frameInfoListSize;	   // 支持的视频帧数量
	VideoFrameInfo* frameInfoList; // 支持的视频帧列表
};

// GetDeviceListReply 获取设备列表响应参数
struct GetDeviceListReply {
	size_t deviceInfoListSize;	// 设备数量
	DeviceInfo* deviceInfoList; // 设备信息列表
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 初始化Becam接口句柄
 *
 * @return Becam接口句柄
 */
_BECAM_API_ BecamHandle BecamNew();

/**
 * @brief 释放Becam接口句柄
 *
 * @param handle Becam接口句柄
 */
_BECAM_API_ void BecamFree(BecamHandle* handle);

/**
 * @brief 获取设备列表
 *
 * @param handle Becam接口句柄
 * @param reply 输出参数
 * @return 状态码
 */
_BECAM_API_ StatusCode BecamGetDeviceList(BecamHandle handle, GetDeviceListReply* reply);

/**
 * @brief 释放设备列表
 *
 * @param handle Becam接口句柄
 * @param input 输入参数
 */
_BECAM_API_ void BecamFreeDeviceList(BecamHandle handle, GetDeviceListReply* input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BECAM_H_ */