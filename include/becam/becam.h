#pragma once

#ifndef _BECAM_H_
#define _BECAM_H_
#ifdef _WIN32
#define _BECAM_API_ __declspec(dllexport)
#else
#define _BECAM_API_
#endif

// 引入必要头文件
#include <stddef.h>
#include <stdint.h>

// Becam接口句柄
typedef void* BecamHandle;

// StatusCode 状态码定义
typedef enum {
	STATUS_CODE_SUCCESS, // 成功
	/**
	 * 通用异常
	 */
	STATUS_CODE_ERR_HANDLE_EMPTY,				 // Becam接口句柄未初始化
	STATUS_CODE_ERR_INPUT_PARAM,				 // 传入参数错误
	STATUS_CODE_ERR_DEVICE_ENUM_FAILED,			 // 设备枚举失败
	STATUS_CODE_ERR_DEVICE_NOT_FOUND,			 // 设备未找到
	STATUS_CODE_ERR_DEVICE_OPEN_FAILED,			 // 设备打开失败
	STATUS_CODE_ERR_DEVICE_NOT_OPEN,			 // 设备未打开
	STATUS_CODE_ERR_DEVICE_FRAME_FMT_SET_FAILED, // 设备视频帧格式配置失败
	STATUS_CODE_ERR_DEVICE_RUN_FAILED,			 // 设备运行失败
	STATUS_CODE_ERR_DEVICE_NOT_RUN,				 // 设备未运行
	STATUS_CODE_ERR_GET_FRAME_FAILED,			 // 获取视频帧失败
	STATUS_CODE_ERR_GET_FRAME_EMPTY,			 // 获取视频帧为空
	/**
	 * Direct Show 异常
	 */
	STATUS_CODE_DSHOW_ERR_INTERNAL_PARAM,		   // DirectShow异常：内部参数错误
	STATUS_CODE_DSHOW_ERR_INIT_COM,				   // DirectShow异常：初始化COM库失败
	STATUS_CODE_DSHOW_ERR_CREATE_ENUMERATOR,	   // DirectShow异常：创建设备枚举器失败
	STATUS_CODE_DSHOW_ERR_GET_DEVICE_PROP,		   // DirectShow异常：获取设备属性失败
	STATUS_CODE_DSHOW_ERR_GET_STREAM_CAPS,		   // DirectShow异常：获取设备流能力失败
	STATUS_CODE_DSHOW_ERR_CREATE_GRAPH_BUILDER,	   // DirectShow异常：创建图像构建器失败
	STATUS_CODE_DSHOW_ERR_ADD_CAPTURE_FILTER,	   // DirectShow异常：添加捕获过滤器到图像构建器失败
	STATUS_CODE_DSHOW_ERR_CREATE_SAMPLE_GRABBER,   // DirectShow异常：创建样品采集器失败
	STATUS_CODE_DSHOW_ERR_GET_SAMPLE_GRABBER_INFC, // DirectShow异常：获取样品采集器接口失败
	STATUS_CODE_DSHOW_ERR_ADD_SAMPLE_GRABBER,	   // DirectShow异常：添加样品采集器到图像构建器失败
	STATUS_CODE_DSHOW_ERR_CREATE_MEDIA_CONTROL,	   // DirectShow异常：创建媒体控制器失败
	STATUS_CODE_DSHOW_ERR_CREATE_NULL_RENDER,	   // DirectShow异常：创建空渲染器失败
	STATUS_CODE_DSHOW_ERR_ADD_NULL_RENDER,		   // DirectShow异常：添加空渲染器到图像构建器失败
	STATUS_CODE_DSHOW_ERR_CAPTURE_GRABBER,		   // DirectShow异常：连接捕获器和采集器失败
	STATUS_CODE_DSHOW_ERR_GRABBER_RENDER,		   // DirectShow异常：连接采集器和渲染器失败
	STATUS_CODE_DSHOW_ERR_FRAME_NOT_UPDATE,		   // DirectShow异常：视频帧未更新
	/**
	 * Media Foundation 异常
	 */
	STATUS_CODE_MF_ERR_CREATE_ATTR_STORE,	   // MediaFoundation异常：创建属性存储器失败
	STATUS_CODE_MF_ERR_SET_ATTR_STORE,		   // MediaFoundation异常：赋值属性存储器失败
	STATUS_CODE_MF_ERR_CREATE_PRESENT_DESC,	   // MediaFoundation异常：获取设备获取演示文稿描述符失败
	STATUS_CODE_MF_ERR_GET_STREAM_DESC,		   // MediaFoundation异常：获取设备视频流的流描述符失败
	STATUS_CODE_MF_ERR_GET_MEDIA_TYPE_HANDLER, // MediaFoundation异常：获取媒体类型处理器失败
	STATUS_CODE_MF_ERR_GET_MEDIA_TYPE_COUNT,   // MediaFoundation异常：获取媒体类型总数量失败
	STATUS_CODE_MF_ERR_GET_MEDIA_TYPE,		   // MediaFoundation异常：获取媒体资源类型失败
	STATUS_CODE_MF_ERR_CONVERT_FRAME_BUFFER,   // MediaFoundation异常：转换视频帧缓冲区失败
	STATUS_CODE_MF_ERR_LOCK_FRAME_BUFFER,	   // MediaFoundation异常：锁定视频帧缓冲区失败
	/**
	 * V4L2 异常
	 */
	STATUS_CODE_V4L2_ERR_REQUEST_BUF, // V4L2异常：申请内核缓冲区失败
	STATUS_CODE_V4L2_ERR_QUERY_BUF,	  // V4L2异常：查询内核缓冲区失败
	STATUS_CODE_V4L2_ERR_MMAP_BUF,	  // V4L2异常：映射内核缓冲区失败
	STATUS_CODE_V4L2_ERR_LOCK_BUF,	  // V4L2异常：缓冲区加锁失败
	STATUS_CODE_V4L2_ERR_UNLOCK_BUF,  // V4L2异常：缓冲区解锁失败
} StatusCode;

// DeviceInfo 设备信息
typedef struct {
	char* name;		  // 设备友好名称
	char* devicePath; // 设备唯一标识符
} DeviceInfo;

// GetDeviceListReply 获取设备列表响应参数
typedef struct {
	size_t deviceInfoListSize;	// 设备数量
	DeviceInfo* deviceInfoList; // 设备信息列表
} GetDeviceListReply;

// VideoFrameCaptureType 视频帧捕获类型
typedef enum {
	VIDEO_FRAME_CAPTURE_TYPE_SP = 1, // 视频帧捕获类型: 单平面
	VIDEO_FRAME_CAPTURE_TYPE_MP = 2, // 视频帧捕获类型: 多平面
} VideoFrameCaptureType;

// VideoFrameSizeType 视频帧分辨率类型
typedef enum {
	VIDEO_FRAME_SIZE_TYPE_DISCRETE = 1,	  // 视频帧分辨率类型: 离散型
	VIDEO_FRAME_SIZE_TYPE_CONTINUOUS = 2, // 视频帧分辨率类型: 连续型
	VIDEO_FRAME_SIZE_TYPE_STEPWISE = 3,	  // 视频帧分辨率类型: 步进型
} VideoFrameSizeType;

// VideoFrameFpsType 视频帧帧率类型
typedef enum {
	VIDEO_FRAME_FPS_TYPE_DISCRETE = 1,	 // 视频帧帧率类型: 离散型
	VIDEO_FRAME_FPS_TYPE_CONTINUOUS = 2, // 视频帧帧率类型: 连续型
	VIDEO_FRAME_FPS_TYPE_STEPWISE = 3,	 // 视频帧帧率类型: 步进型
} VideoFrameFpsType;

// VideoFrameFpsInfoUnion 视频帧离散型帧率信息
typedef struct {
	uint32_t numerator;	  // 一帧消耗的时间分子（单位：秒）
	uint32_t denominator; // 一帧消耗的时间分母（单位：秒）
} VideoFrameDiscreteFpsInfo;

// VideoFrameFpsInfoUnion 视频帧连续型、步进型帧率信息
typedef struct {
	uint32_t minNumerator;	  // 一帧消耗的时间最小分子（单位：秒）
	uint32_t minDenominator;  // 一帧消耗的时间最小分母（单位：秒）
	uint32_t maxNumerator;	  // 一帧消耗的时间最大分子（单位：秒）
	uint32_t maxDenominator;  // 一帧消耗的时间最大分母（单位：秒）
	uint32_t stepNumerator;	  // 步进时间分子（单位：秒）
	uint32_t stepDenominator; // 步进时间分母（单位：秒）
} VideoFrameStepwiseFpsInfo;

// VideoFrameFpsInfoUnion 视频帧帧率类型信息
typedef union {
	VideoFrameDiscreteFpsInfo discrete; // 离散型帧率信息
	VideoFrameStepwiseFpsInfo stepwise; // 连续型、步进型帧率信息
} VideoFrameFpsInfoUnion;

// VideoFrameFpsInfo 视频帧帧率信息
typedef struct {
	VideoFrameFpsType fpsType;		// 帧率类型
	VideoFrameFpsInfoUnion fpsInfo; // 帧率信息
} VideoFrameFpsInfo;

// VideoFrameSizeInfo 视频帧离散型分辨率信息
typedef struct {
	uint32_t width;					// 分辨率宽度
	uint32_t height;				// 分辨率高度
	size_t fpsInfoListSize;			// 帧率信息列表大小
	VideoFrameFpsInfo* fpsInfoList; // 帧率信息列表
} VideoFrameDiscreteSizeInfo;

// VideoFrameSizeInfo 视频帧连续型、步进型分辨率信息
typedef struct {
	uint32_t minWidth;				// 分辨率最小宽度
	uint32_t maxWidth;				// 分辨率最大宽度
	uint32_t stepWidth;				// 分辨率步进宽度
	uint32_t minHeight;				// 分辨率最小高度
	uint32_t maxHeight;				// 分辨率最大高度
	uint32_t stepHeight;			// 分辨率步进高度
	size_t fpsInfoListSize;			// 帧率信息列表大小
	VideoFrameFpsInfo* fpsInfoList; // 帧率信息列表
} VideoFrameStepwiseSizeInfo;

// VideoFrameSizeInfo 视频帧分辨率类型信息
typedef union {
	VideoFrameDiscreteSizeInfo discrete; // 离散型分辨率信息
	VideoFrameStepwiseSizeInfo stepwise; // 连续型、步进型分辨率信息
} VideoFrameSizeInfoUnion;

// VideoFrameSizeInfo 视频帧分辨率信息
typedef struct {
	VideoFrameSizeType sizeType;	  // 分辨率类型
	VideoFrameSizeInfoUnion sizeInfo; // 分辨率类型信息
} VideoFrameSizeInfo;

// VideoFrameInfo 视频帧信息
typedef struct {
	uint32_t format;				  // 格式（FOURCC表示）
	VideoFrameCaptureType capType;	  // 捕获类型
	size_t sizeInfoListSize;		  // 分辨率信息列表大小
	VideoFrameSizeInfo* sizeInfoList; // 分辨率信息列表
} VideoFrameInfo;

// GetDeviceConfigListReply 获取设备配置列表响应参数
typedef struct {
	size_t videoFrameInfoListSize;		// 视频帧信息数量
	VideoFrameInfo* videoFrameInfoList; // 视频帧信息列表
} GetDeviceConfigListReply;

// VideoFrameCaptureInfo 视频帧捕获信息
typedef struct {
	uint32_t format;			   // 格式（FOURCC表示）
	VideoFrameCaptureType capType; // 捕获类型
	uint32_t width;				   // 分辨率宽度
	uint32_t height;			   // 分辨率高度
	uint32_t numerator;			   // 一帧消耗的时间分子（单位：秒）
	uint32_t denominator;		   // 一帧消耗的时间分母（单位：秒）
} VideoFrameCaptureInfo;

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
 * @brief 获取设备配置列表
 *
 * @param handle Becam接口句柄
 * @param device 选中的设备
 * @param reply 输出参数
 * @return 状态码
 */
_BECAM_API_ StatusCode BecamGetDeviceConfigList(BecamHandle handle, const char* devicePath, GetDeviceConfigListReply* reply);

/**
 * @brief 释放设备配置列表
 *
 * @param handle Becam接口句柄
 * @param input 输入参数
 */
_BECAM_API_ void BecamFreeDeviceConfigList(BecamHandle handle, GetDeviceConfigListReply* input);

/**
 * @brief 打开设备
 *
 * @param handle Becam接口句柄
 * @param devicePath 设备路径
 * @param frameCaptureInfo 视频帧捕获信息
 * @return 状态码
 */
_BECAM_API_ StatusCode BecamOpenDevice(BecamHandle handle, const char* devicePath, const VideoFrameCaptureInfo* frameCaptureInfo);

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