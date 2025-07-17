#pragma once

#ifndef _BECAM_H_
#define _BECAM_H_

// 导入或导出动态库
#if defined(BECAM_SHARED) || defined(BECAM_SHARED_EXPORT)
/*********************************** ↓↓↓ 动态库导入导出配置 ↓↓↓ ***********************************/
// 区分Windows和Unix
#ifdef _WIN32
/************************* ↓↓↓ Windows 动态库导入导出配置 ↓↓↓ *************************/
// 区分导入和导出
#ifdef BECAM_SHARED_EXPORT
#define BECAM_API __declspec(dllexport) // 导出Windows动态库
#else
#define BECAM_API __declspec(dllimport) // 导入Windows动态库
#endif
/************************* ↑↑↑ Windows 动态库导入导出配置 ↑↑↑ *************************/
#else
/************************* ↓↓↓ 类 Unix 动态库导入导出配置 ↓↓↓ *************************/
// 区分导入和导出
#ifdef BECAM_SHARED_EXPORT
#define BECAM_API __attribute__((visibility("default"))) // 导出Unix动态库
#else
#define BECAM_API // 导入Unix动态库
#endif
/************************* ↑↑↑ 类 Unix 动态库导入导出配置 ↑↑↑ *************************/
#endif
/*********************************** ↑↑↑ 动态库导入导出配置 ↑↑↑ ***********************************/
#else
/*********************************** ↓↓↓ 静态库导入导出配置 ↓↓↓ ***********************************/
#define BECAM_API // 导入或导出静态库
/*********************************** ↑↑↑ 静态库导入导出配置 ↑↑↑ ***********************************/
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
	STATUS_CODE_ERR_DEVICE_FRAME_FMT_NOT_FOUND,	 // 设备视频帧格式未找到
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

// VideoFrameInfo 视频帧信息
typedef struct {
	uint32_t format; // 格式（FOURCC表示）
	uint32_t width;	 // 分辨率宽度
	uint32_t height; // 分辨率高度
	uint32_t fps;	 // 分辨率帧率
} VideoFrameInfo;

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

// GetDeviceConfigListReply 获取设备配置列表响应参数
typedef struct {
	size_t videoFrameInfoListSize;		// 视频帧信息数量
	VideoFrameInfo* videoFrameInfoList; // 视频帧信息列表
} GetDeviceConfigListReply;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 初始化Becam接口句柄
 * @return Becam接口句柄
 */
BECAM_API BecamHandle BecamNew();

/**
 * @brief 释放Becam接口句柄
 * @param handle [in] Becam接口句柄
 */
BECAM_API void BecamFree(BecamHandle* handle);

/**
 * @brief 获取设备列表
 * @param handle [in] Becam接口句柄
 * @param reply [out] 输出参数
 * @return 状态码 @ref(StatusCode)
 */
BECAM_API StatusCode BecamGetDeviceList(const BecamHandle handle, GetDeviceListReply* reply);

/**
 * @brief 释放设备列表
 * @param input [in] 输入参数
 */
BECAM_API void BecamFreeDeviceList(GetDeviceListReply* input);

/**
 * @brief 获取设备配置列表
 * @param handle [in] Becam接口句柄
 * @param device [in] 选中的设备
 * @param reply [out] 输出参数
 * @return 状态码 @ref(StatusCode)
 */
BECAM_API StatusCode BecamGetDeviceConfigList(const BecamHandle handle, const char* devicePath, GetDeviceConfigListReply* reply);

/**
 * @brief 释放设备配置列表
 * @param input [in] 输入参数
 */
BECAM_API void BecamFreeDeviceConfigList(GetDeviceConfigListReply* input);

/**
 * @brief 打开设备
 * @param handle [in] Becam接口句柄
 * @param devicePath [in] 设备路径
 * @param frameInfo [in] 视频帧信息
 * @return 状态码 @ref(StatusCode)
 */
BECAM_API StatusCode BecamOpenDevice(const BecamHandle handle, const char* devicePath, const VideoFrameInfo* frameInfo);

/**
 * @brief 关闭设备
 * @param handle [in] Becam接口句柄
 */
BECAM_API void BecamCloseDevice(const BecamHandle handle);

/**
 * @brief 获取视频帧
 * @param handle [in] Becam接口句柄
 * @param data [out] 视频帧流
 * @param size [out] 视频帧流大小
 * @return 状态码 @ref(StatusCode)
 */
BECAM_API StatusCode BecamGetFrame(const BecamHandle handle, uint8_t** data, size_t* size);

/**
 * @brief 释放视频帧
 * @param data [in] 视频帧流
 */
BECAM_API void BecamFreeFrame(uint8_t** data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BECAM_H_ */