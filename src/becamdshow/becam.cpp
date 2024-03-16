#include "BecamDirectShow.hpp"
#include <becam/becam.h>

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
void BecamFree(BecamHandle* handle) {
	// 检查参数
	if (handle == nullptr || *handle == nullptr) {
		// 参数错误
		return;
	}

	// 获取Becam接口句柄
	auto becamHandle = static_cast<BecamDirectShow*>(*handle);
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
StatusCode BecamGetDeviceList(BecamHandle handle, GetDeviceListReply* reply) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 获取相机列表
	return becamHandle->GetDeviceList(reply);
}

/**
 * @brief 释放设备列表
 *
 * @param handle Becam接口句柄
 * @param input 输入参数
 */
void BecamFreeDeviceList(BecamHandle handle, GetDeviceListReply* input) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 执行相机列表释放
	becamHandle->FreeDeviceList(input);
}

/**
 * @brief 打开设备
 *
 * @param handle Becam接口句柄
 * @param devicePath 设备路径
 * @param frameInfo 视频帧信息
 * @return 状态码
 */
StatusCode BecamOpenDevice(BecamHandle handle, const char* devicePath, const VideoFrameInfo* frameInfo) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 执行相机打开
	return becamHandle->OpenDevice(devicePath, frameInfo);
}

/**
 * @brief 关闭设备
 *
 * @param handle Becam接口句柄
 */
void BecamCloseDevice(BecamHandle handle) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 执行相机关闭
	becamHandle->CloseDevice();
}

/**
 * @brief 获取视频帧
 *
 * @param handle Becam接口句柄
 * @param data 视频帧流
 * @param size 视频帧流大小
 * @return 状态码
 */
StatusCode BecamGetFrame(BecamHandle handle, uint8_t** data, size_t* size) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 执行获取视频帧
	return becamHandle->GetFrame(data, size);
}

/**
 * @brief 释放视频帧
 *
 * @param handle Becam接口句柄
 * @param data 视频帧流
 */
void BecamFreeFrame(BecamHandle handle, uint8_t** data) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}

	// 转换句柄类型
	BecamDirectShow* becamHandle = static_cast<BecamDirectShow*>(handle);
	// 执行释放视频帧
	becamHandle->FreeFrame(data);
}
