#include "BecamUVC.hpp"
#include <becam/becam.h>

/**
 * @implements 实现初始化Becam接口句柄
 */
BecamHandle BecamNew() {
	// 创建Becam接口句柄
	return new BecamUVC();
}

/**
 * @implements 实现释放Becam接口句柄
 */
void BecamFree(BecamHandle* handle) {
	// 检查参数
	if (handle == nullptr || *handle == nullptr) {
		return;
	}

	// 获取Becam接口句柄
	auto becamHandle = static_cast<BecamUVC*>(*handle);
	// 释放Becam接口句柄
	delete becamHandle;

	// 置空
	becamHandle = nullptr;
	*handle = nullptr;
}

/**
 * @implements 实现获取设备列表
 */
StatusCode BecamGetDeviceList(BecamHandle handle, GetDeviceListReply* reply) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}
	// 检查参数
	if (reply == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}
	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 获取相机列表
	return becamHandle->GetDeviceList(*reply);
}

/**
 * @implements 实现释放设备列表
 */
void BecamFreeDeviceList(BecamHandle handle, GetDeviceListReply* input) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}
	// 检查参数
	if (input == nullptr) {
		return;
	}
	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行相机列表释放
	becamHandle->FreeDeviceList(*input);
}

/**
 * @implements 实现获取设备配置列表
 */
StatusCode BecamGetDeviceConfigList(BecamHandle handle, const char* devicePath, GetDeviceConfigListReply* reply) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}
	// 检查参数
	if (devicePath == nullptr || reply == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}
	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行获取设备配置列表
	return becamHandle->GetDeviceConfigList(devicePath, *reply);
}

/**
 * @implements 实现释放设备配置列表
 */
void BecamFreeDeviceConfigList(BecamHandle handle, GetDeviceConfigListReply* input) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}
	// 检查参数
	if (input == nullptr) {
		return;
	}
	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行相机配置列表释放
	becamHandle->FreeDeviceConfigList(*input);
}

/**
 * @implements 实现打开设备
 */
StatusCode BecamOpenDevice(BecamHandle handle, const char* devicePath, const VideoFrameInfo* frameInfo) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}
	// 检查参数
	if (devicePath == nullptr || frameInfo == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行相机打开
	return becamHandle->OpenDevice(devicePath, *frameInfo);
}

/**
 * @implements 实现关闭设备
 */
void BecamCloseDevice(BecamHandle handle) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}

	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行相机关闭
	becamHandle->CloseDevice();
}

/**
 * @implements 实现获取视频帧
 */
StatusCode BecamGetFrame(BecamHandle handle, uint8_t** data, size_t* size) {
	// 检查句柄
	if (handle == nullptr) {
		return StatusCode::STATUS_CODE_ERR_HANDLE_EMPTY;
	}
	// 检查参数
	if (data == nullptr || size == nullptr) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行获取视频帧
	return becamHandle->GetFrame(*data, *size);
}

/**
 * @implements 实现释放视频帧
 */
void BecamFreeFrame(BecamHandle handle, uint8_t** data) {
	// 检查句柄
	if (handle == nullptr) {
		return;
	}
	// 检查参数
	if (data == nullptr) {
		return;
	}

	// 转换句柄类型
	BecamUVC* becamHandle = static_cast<BecamUVC*>(handle);
	// 执行释放视频帧
	becamHandle->FreeFrame(*data);
}
