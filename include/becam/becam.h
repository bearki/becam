#pragma once

#ifndef _BECAM_H_
#define _BECAM_H_
#define _BECAM_API_ __declspec(dllexport)

// 引入必要头文件
#include <stdint.h>

// Becam接口句柄
typedef void* BecamHandle;

// StatusCode 状态码定义
typedef enum {
	STATUS_CODE_SUCCESS,					 // 成功
	STATUS_CODE_NOT_FOUND_DEVICE,			 // 未找到设备
	STATUS_CODE_ERR_HANDLE_EMPTY,			 // Becam接口句柄未初始化
	STATUS_CODE_ERR_INPUT_PARAM,			 // 传入参数错误
	STATUS_CODE_ERR_INTERNAL_PARAM,			 // 内部参数错误
	STATUS_CODE_ERR_INIT_COM,				 // 初始化COM库失败
	STATUS_CODE_ERR_CREATE_ENUMERATOR,		 // 创建设备枚举器失败
	STATUS_CODE_ERR_DEVICE_ENUM,			 // 设备枚举失败
	STATUS_CODE_ERR_GET_DEVICE_PROP,		 // 获取设备属性失败
	STATUS_CODE_ERR_GET_STREAM_CAPS,		 // 获取设备流能力失败
	STATUS_CODE_ERR_NOMATCH_STREAM_CAPS,	 // 未匹配到流能力
	STATUS_CODE_ERR_SET_MEDIA_TYPE,			 // 设置媒体类型失败
	STATUS_CODE_ERR_SELECTED_DEVICE,		 // 选择设备失败
	STATUS_CODE_ERR_CREATE_GRAPH_BUILDER,	 // 创建图像构建器失败
	STATUS_CODE_ERR_ADD_CAPTURE_FILTER,		 // 添加捕获过滤器到图像构建器失败
	STATUS_CODE_ERR_CREATE_SAMPLE_GRABBER,	 // 创建样品采集器失败
	STATUS_CODE_ERR_GET_SAMPLE_GRABBER_INFC, // 获取样品采集器接口失败
	STATUS_CODE_ERR_ADD_SAMPLE_GRABBER,		 // 添加样品采集器到图像构建器失败
	STATUS_CODE_ERR_CREATE_MEDIA_CONTROL,	 // 创建媒体控制器失败
	STATUS_CODE_ERR_CREATE_NULL_RENDER,		 // 创建空渲染器失败
	STATUS_CODE_ERR_ADD_NULL_RENDER,		 // 添加空渲染器到图像构建器失败
	STATUS_CODE_ERR_CAPTURE_GRABBER,		 // 连接捕获器和采集器失败
	STATUS_CODE_ERR_GRABBER_RENDER,			 // 连接采集器和渲染器失败
	STATUS_CODE_ERR_DEVICE_NOT_OPEN,		 // 设备未打开
	STATUS_CODE_ERR_FRAME_EMPTY,			 // 视频帧为空
	STATUS_CODE_ERR_FRAME_NOT_UPDATE,		 // 视频帧未更新
} StatusCode;

// VideoFrameInfo 视频帧信息
typedef struct {
	uint32_t format; // 格式
	uint32_t width;	 // 分辨率宽度
	uint32_t height; // 分辨率高度
	uint32_t fps;	 // 分辨率帧率
} VideoFrameInfo;

// DeviceInfo 设备信息
typedef struct {
	char* name;					   // 设备友好名称
	char* devicePath;			   // 设备路径
	char* locationInfo;			   // 设备位置信息
	size_t frameInfoListSize;	   // 支持的视频帧数量
	VideoFrameInfo* frameInfoList; // 支持的视频帧列表
} DeviceInfo;

// GetDeviceListReply 获取设备列表响应参数
typedef struct {
	size_t deviceInfoListSize;	// 设备数量
	DeviceInfo* deviceInfoList; // 设备信息列表
} GetDeviceListReply;

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

/**
 * @brief 打开设备
 *
 * @param handle Becam接口句柄
 * @param devicePath 设备路径
 * @param frameInfo 视频帧信息
 * @return 状态码
 */
_BECAM_API_ StatusCode BecamOpenDevice(BecamHandle handle, const char* devicePath, const VideoFrameInfo* frameInfo);

/**
 * @brief 关闭设备
 *
 * @param handle Becam接口句柄
 */
_BECAM_API_ void BecamCloseDevice(BecamHandle handle);

/**
 * @brief 获取视频帧
 *
 * @param handle Becam接口句柄
 * @param data 视频帧流
 * @param size 视频帧流大小
 * @return 状态码
 */
_BECAM_API_ StatusCode BecamGetFrame(BecamHandle handle, uint8_t** data, size_t* size);

/**
 * @brief 释放视频帧
 *
 * @param handle Becam接口句柄
 * @param data 视频帧流
 */
_BECAM_API_ void BecamFreeFrame(BecamHandle handle, uint8_t** data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BECAM_H_ */