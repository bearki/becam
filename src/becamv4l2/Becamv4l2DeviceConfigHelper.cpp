#include "Becamv4l2DeviceConfigHelper.hpp"
#include "xioctl.hpp"
#include <string.h>

/**
 * @implements 实现构造函数
 */
Becamv4l2DeviceConfigHelper::Becamv4l2DeviceConfigHelper(const int fd) {
	// 赋值设备句柄
	this->deviceFdHandle = fd;
}

/**
 * @implements 实现析构函数
 */
Becamv4l2DeviceConfigHelper::~Becamv4l2DeviceConfigHelper() {}

/**
 * @implements 实现获取设备支持的视频帧帧率列表
 */
void Becamv4l2DeviceConfigHelper::getVideoFrameFpsList(v4l2_frmivalenum frmival, VideoFrameFpsInfo*& outList, size_t& outListSize) {
	// 声明临时Vector
	std::vector<VideoFrameFpsInfo> fpsList;
	// 枚举指定格式，指定分辨率支持的所有帧率
	while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
		// 忽略无效帧率
		if ((frmival.type == v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_DISCRETE && frmival.discrete.denominator <= 0) ||
			(frmival.type == v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_CONTINUOUS && frmival.stepwise.max.denominator <= 0) ||
			(frmival.type == v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_STEPWISE && frmival.stepwise.max.denominator <= 0)) {
			// 叠加帧率枚举下标
			frmival.index++;
			// 继续下一个
			continue;
		}

		// 构建帧率信息
		VideoFrameFpsInfo fpsItem;
		// 区分帧率类型
		switch (frmival.type) {
			// （离散型）​表示设备支持固定单一帧率，每个索引对应一个唯一的帧间隔值
			case v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_DISCRETE: {
				// 构建离散型帧率信息
				VideoFrameDiscreteFpsInfo tmp = {0};
				tmp.numerator = frmival.discrete.numerator;
				tmp.denominator = frmival.discrete.denominator;
				// 赋值帧率信息帧率类型
				fpsItem.fpsType = VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_DISCRETE;
				fpsItem.fpsInfo.discrete = tmp;
				break;
			}

			// （连续型）​​是STEPWISE的特例，表示设备支持连续可调帧率，且步长固定为1
			// （步进型）​表示设备支持可调整步进的帧率范围，需通过min（最小帧率）、max（最大帧率）和step（步长）三个字段描述
			case v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_CONTINUOUS:
			case v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_STEPWISE: {
				// 构建离散型帧率信息
				VideoFrameStepwiseFpsInfo tmp = {0};
				tmp.minNumerator = frmival.stepwise.min.numerator;
				tmp.minDenominator = frmival.stepwise.min.denominator;
				tmp.maxNumerator = frmival.stepwise.max.numerator;
				tmp.maxDenominator = frmival.stepwise.max.denominator;
				tmp.stepNumerator = frmival.stepwise.step.numerator;
				tmp.stepDenominator = frmival.stepwise.step.denominator;
				// 赋值帧率信息帧率类型
				if (frmival.type == v4l2_frmivaltypes::V4L2_FRMIVAL_TYPE_CONTINUOUS) {
					fpsItem.fpsType = VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_CONTINUOUS;
				} else {
					fpsItem.fpsType = VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_STEPWISE;
				}
				fpsItem.fpsInfo.stepwise = tmp;
				break;
			}
		}

		// 添加到临时列表
		fpsList.push_back(fpsItem);
		// 叠加帧率枚举下标
		frmival.index++;
	}

	// 是否需要拷贝结果
	outListSize = fpsList.size();
	if (outListSize > 0) {
		outList = new VideoFrameFpsInfo[outListSize];
		memcpy(outList, fpsList.data(), sizeof(VideoFrameFpsInfo) * outListSize);
	}
}

/**
 * @implements 实现释放设备支持的视频帧帧率列表
 */
void Becamv4l2DeviceConfigHelper::freeVideoFrameFpsList(VideoFrameFpsInfo*& outList, size_t& outListSize) {
	if (outList == nullptr || outListSize <= 0) {
		return;
	}
	// 例表中的元素无引用指针，可直接释放列表
	delete[] outList;
	outList = nullptr;
	outListSize = 0;
}

/**
 * @implements 实现获取设备支持的视频帧分辨率列表
 */
void Becamv4l2DeviceConfigHelper::getVideoFrameSizeList(v4l2_frmsizeenum frmsize, VideoFrameSizeInfo*& outList, size_t& outListSize) {
	// 声明临时Vector
	std::vector<VideoFrameSizeInfo> sizeList;
	// 执行遍历
	while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
		// 忽略无效分辨率
		if ((frmsize.type == v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_DISCRETE &&
			 (frmsize.discrete.width <= 0 || frmsize.discrete.height <= 0)) ||
			(frmsize.type == v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_CONTINUOUS &&
			 (frmsize.stepwise.max_width <= 0 || frmsize.stepwise.max_height <= 0)) ||
			(frmsize.type == v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_STEPWISE &&
			 (frmsize.stepwise.max_width <= 0 || frmsize.stepwise.max_height <= 0))) {
			// 分辨率枚举下标叠加
			frmsize.index++;
			// 继续下一个
			continue;
		}

		// 构建结果集元素
		VideoFrameSizeInfo sizeItem;
		// 区分分辨率类型
		switch (frmsize.type) {
			// （离散型）设备支持固定离散的分辨率列表
			case v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_DISCRETE: {
				// 赋值结果集元素分辨率信息
				VideoFrameDiscreteSizeInfo tmp = {0};
				tmp.width = frmsize.discrete.width;
				tmp.height = frmsize.discrete.height;

				// 查询支持的帧率
				v4l2_frmivalenum frmival = {0};
				frmival.index = 0;							 // 初始下标为0
				frmival.pixel_format = frmsize.pixel_format; // 指定要枚举的格式
				frmival.width = frmsize.discrete.width;		 // 指定要枚举的分辨率
				frmival.height = frmsize.discrete.height;	 // 指定要枚举的分辨率
				this->getVideoFrameFpsList(frmival, tmp.fpsInfoList, tmp.fpsInfoListSize);
				// 丢弃帧率列表为空的
				if (tmp.fpsInfoListSize <= 0 || tmp.fpsInfoList == nullptr) {
					// 分辨率枚举下标叠加
					frmsize.index++;
					// 继续下一个
					continue;
				}

				// 赋值结果
				sizeItem.sizeType = VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_DISCRETE;
				sizeItem.sizeInfo.discrete = tmp;
				break;
			}

			// （连续型）​设备支持连续可变的分辨率范围
			// min：最小分辨率, max：最大分辨率, step：步长固定为1
			// （步进型）​设备支持分步调整的分辨率范围，需通过三个字段描述
			// min：最小分辨率, max：最大分辨率, step：步长
			case v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_CONTINUOUS:
			case v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_STEPWISE: {
				// 赋值连续型分辨率信息
				VideoFrameStepwiseSizeInfo tmp = {0};
				tmp.minWidth = frmsize.stepwise.min_width;
				tmp.maxWidth = frmsize.stepwise.max_width;
				tmp.stepWidth = frmsize.stepwise.step_width;
				tmp.minHeight = frmsize.stepwise.min_height;
				tmp.maxHeight = frmsize.stepwise.max_height;
				tmp.stepHeight = frmsize.stepwise.step_height;

				// 查询支持的帧率
				v4l2_frmivalenum frmival = {0};
				frmival.index = 0;							 // 初始下标为0
				frmival.pixel_format = frmsize.pixel_format; // 指定要枚举的格式
				this->getVideoFrameFpsList(frmival, tmp.fpsInfoList, tmp.fpsInfoListSize);
				// 丢弃帧率列表为空的分辨率
				if (tmp.fpsInfoListSize <= 0 || tmp.fpsInfoList == nullptr) {
					// 分辨率枚举下标叠加
					frmsize.index++;
					// 继续下一个
					continue;
				}

				// 赋值结果
				if (frmsize.type == v4l2_frmsizetypes::V4L2_FRMSIZE_TYPE_CONTINUOUS) {
					sizeItem.sizeType = VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_CONTINUOUS;
				} else {
					sizeItem.sizeType = VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_STEPWISE;
				}
				sizeItem.sizeInfo.stepwise = tmp;
				break;
			}
		}

		// 添加到分辨率Vector
		sizeList.push_back(sizeItem);
		// 分辨率枚举下标叠加
		frmsize.index++;
	}

	// 是否需要拷贝分辨率列表
	outListSize = sizeList.size();
	if (outListSize > 0) {
		outList = new VideoFrameSizeInfo[outListSize];
		memcpy(outList, sizeList.data(), sizeof(VideoFrameSizeInfo) * outListSize);
	}
}

/**
 * @implements 实现释放设备支持的视频帧分辨率列表
 */
void Becamv4l2DeviceConfigHelper::freeVideoFrameSizeList(VideoFrameSizeInfo*& outList, size_t& outListSize) {
	// 检查
	if (outList == nullptr || outListSize <= 0) {
		return;
	}
	// 每个分辨率内部有帧率列表，需要遍历处理
	for (size_t i = 0; i < outListSize; i++) {
		// 当前元素
		auto item = outList[i];
		// 区分分辨率类型
		switch (item.sizeType) {
				// 释放离散型分辨率的帧率列表
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_DISCRETE: {
				// 释放分辨率
				Becamv4l2DeviceConfigHelper::freeVideoFrameFpsList(item.sizeInfo.discrete.fpsInfoList,
																   item.sizeInfo.discrete.fpsInfoListSize);
				break;
			}

			// 释放连续型或步进型分辨率的帧率列表
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_CONTINUOUS:
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_STEPWISE: {
				// 释放分辨率
				Becamv4l2DeviceConfigHelper::freeVideoFrameFpsList(item.sizeInfo.stepwise.fpsInfoList,
																   item.sizeInfo.stepwise.fpsInfoListSize);
				break;
			}
		}
	}
	// 释放列表
	delete[] outList;
	outList = nullptr;
	outListSize = 0;
}

/**
 * @implements 实现获取设备支持的视频帧格式列表
 */
void Becamv4l2DeviceConfigHelper::getVideoFrameFormatList(v4l2_fmtdesc fmt, std::vector<VideoFrameInfo>& fmtList) {
	// 枚举支持的像素格式
	while (xioctl(this->deviceFdHandle, VIDIOC_ENUM_FMT, &fmt) == 0) {
		// 构建视频帧信息
		VideoFrameInfo fmtItem = {0};
		fmtItem.format = fmt.pixelformat;
		if (fmt.type == v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE) {
			fmtItem.capType = VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_SP;
		} else if (fmt.type == v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			fmtItem.capType = VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_MP;
		}
		fmtItem.sizeInfoListSize = 0;
		fmtItem.sizeInfoList = nullptr;

		// 枚举当前格式下支持的分辨率
		v4l2_frmsizeenum frmsize = {0};
		frmsize.index = 0;						// 初始下标为0
		frmsize.pixel_format = fmt.pixelformat; // 指定要枚举的格式
		this->getVideoFrameSizeList(frmsize, fmtItem.sizeInfoList, fmtItem.sizeInfoListSize);
		// 丢弃分辨率列表为空的格式
		if (fmtItem.sizeInfoListSize <= 0 || fmtItem.sizeInfoList == nullptr) {
			// 叠加格式枚举下标
			fmt.index++;
			// 继续下一个
			continue;
		}

		// 追加结果
		fmtList.push_back(fmtItem);
		// 叠加格式枚举下标
		fmt.index++;
	}
}

/**
 * @implements 实现释放设备支持的视频帧格式列表
 */
void Becamv4l2DeviceConfigHelper::freeVideoFrameFormatList(VideoFrameInfo*& outList, size_t& outListSize) {
	// 检查
	if (outList == nullptr || outListSize <= 0) {
		return;
	}

	// 遍历格式列表
	for (size_t i = 0; i < outListSize; i++) {
		// 获取单个格式元素
		auto item = outList[i];
		// 释放分辨率列表
		Becamv4l2DeviceConfigHelper::freeVideoFrameSizeList(item.sizeInfoList, item.sizeInfoListSize);
	}

	// 释放格式列表
	delete[] outList;
	outList = nullptr;
	outListSize = 0;
}

/**
 * @implements 实现获取设备支持的配置列表
 */
StatusCode Becamv4l2DeviceConfigHelper::GetDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize) {
	// 重置
	reply = nullptr;
	replySize = 0;
	// 临时配置信息列表
	std::vector<VideoFrameInfo> fmtList;

	// 获取单平面捕获支持的配置
	v4l2_fmtdesc fmt1 = {0};
	fmt1.index = 0;											// 初始下标为0
	fmt1.type = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE; // 指定要枚举的流类型为视频捕获（单平面、多平面都可以）
	this->getVideoFrameFormatList(fmt1, fmtList);
	// 获取多平面捕获支持的配置
	v4l2_fmtdesc fmt2 = {0};
	fmt2.index = 0;												   // 初始下标为0
	fmt2.type = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE; // 指定要枚举的流类型为视频捕获（单平面、多平面都可以）
	this->getVideoFrameFormatList(fmt2, fmtList);

	// 是否需要赋值响应结果
	replySize = fmtList.size();
	if (replySize > 0) {
		// 拷贝配置列表
		reply = new VideoFrameInfo[replySize];
		memcpy(reply, fmtList.data(), sizeof(VideoFrameInfo) * replySize);
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放已获取的设备支持的配置列表
 */
void Becamv4l2DeviceConfigHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 检查
	if (input == nullptr || inputSize == 0) {
		return;
	}
	// 执行释放
	Becamv4l2DeviceConfigHelper::freeVideoFrameFormatList(input, inputSize);
}
