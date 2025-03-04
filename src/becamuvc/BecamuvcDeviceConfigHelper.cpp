#include "BecamuvcDeviceConfigHelper.hpp"
#include <linux/videodev2.h>
#include <string.h>
#include <vector>

/**
 * @implements 实现构造函数
 */
BecamuvcDeviceConfigHelper::BecamuvcDeviceConfigHelper(const int fd) {
	// 赋值设备句柄
	this->deviceFdHandle = fd;
}

/**
 * @implements 实现析构函数
 */
BecamuvcDeviceConfigHelper::~BecamuvcDeviceConfigHelper() {}

/**
 * @implements 实现获取设备支持的配置列表
 */
StatusCode BecamuvcDeviceConfigHelper::GetDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize) {
	// // 重置
	// reply = nullptr;
	// replySize = 0;

	// // 临时配置信息列表
	// std::vector<VideoFrameInfo> configList;

	// // 提取到的格式信息
	// uvc_fmtdesc fmt = {0};
	// fmt.index = 0;							// 初始下标为0
	// fmt.type = UVC_BUF_TYPE_VIDEO_CAPTURE; // 指定要枚举的流类型为视频捕获
	// // 枚举支持的像素格式
	// while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FMT, &fmt) == 0) {
	// 	// 枚举当前格式下支持的分辨率
	// 	uvc_frmsizeenum frmsize = {0};
	// 	frmsize.index = 0;						// 初始下标为0
	// 	frmsize.pixel_format = fmt.pixelformat; // 指定要枚举的格式
	// 	while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
	// 		// 枚举当前分辨率下支持的帧率
	// 		uvc_frmivalenum frmival = {0};
	// 		frmival.index = 0;							 // 初始下标为0
	// 		frmival.pixel_format = frmsize.pixel_format; // 指定要枚举的格式
	// 		frmival.width = frmsize.discrete.width;		 // 指定要枚举的分辨率
	// 		frmival.height = frmsize.discrete.height;	 // 指定要枚举的分辨率
	// 		while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
	// 			// 构建视频帧信息
	// 			VideoFrameInfo videoFrameInfo = {0};
	// 			videoFrameInfo.format = frmival.pixel_format;
	// 			videoFrameInfo.width = frmival.width;
	// 			videoFrameInfo.height = frmival.height;
	// 			videoFrameInfo.fps = frmival.discrete.denominator / frmival.discrete.numerator;

	// 			// 添加到临时列表
	// 			configList.push_back(videoFrameInfo);

	// 			// 叠加帧率枚举下标
	// 			frmival.index++;
	// 		}

	// 		// 叠加分辨率枚举下标
	// 		frmsize.index++;
	// 	}

	// 	// 叠加格式枚举下标
	// 	fmt.index++;
	// }

	// // 是否需要赋值响应结果
	// if (configList.size() > 0) {
	// 	// 拷贝配置列表
	// 	replySize = configList.size();
	// 	reply = new VideoFrameInfo[replySize];
	// 	memcpy(reply, configList.data(), replySize * sizeof(VideoFrameInfo));
	// }

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放已获取的设备支持的配置列表
 */
void BecamuvcDeviceConfigHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 检查
	if (input == nullptr || inputSize == 0) {
		return;
	}
	// 释放
	delete[] input;
	input = nullptr;
	inputSize = 0;
}
