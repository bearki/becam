#include <becam/becam.h>
#include <iostream>
#include <linux/videodev2.h>

static std::string fourccToStringSafe(uint32_t fourcc) {
	char buffer[5];
	buffer[3] = (static_cast<char>((fourcc >> 24) & 0xFF)) >= 32 ? static_cast<char>((fourcc >> 24) & 0xFF) : '.';
	buffer[2] = (static_cast<char>((fourcc >> 16) & 0xFF)) >= 32 ? static_cast<char>((fourcc >> 16) & 0xFF) : '.';
	buffer[1] = (static_cast<char>((fourcc >> 8) & 0xFF)) >= 32 ? static_cast<char>((fourcc >> 8) & 0xFF) : '.';
	buffer[0] = (static_cast<char>(fourcc & 0xFF)) >= 32 ? static_cast<char>(fourcc & 0xFF) : '.';
	buffer[4] = '\0';
	return std::string(buffer);
}

// 打印视频帧帧率信息
static void printVideoFrameFpsInfo(size_t listSize, VideoFrameFpsInfo* list) {
	// 遍历所有帧率
	for (size_t i = 0; i < listSize; i++) {
		// 获取帧率信息
		auto item = list[i];
		// 区分帧率类型
		switch (item.fpsType) {
			// 离散型
			case VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_DISCRETE: {
				// 获取离散型帧率信息
				auto info = item.fpsInfo.discrete;
				std::cout << "\t\t\tFPS: " << info.numerator << "/" << info.denominator << std::endl;
				break;
			}

			// 连续型、步进型
			case VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_CONTINUOUS:
			case VideoFrameFpsType::VIDEO_FRAME_FPS_TYPE_STEPWISE: {
				// 获取连续型、步进型帧率信息
				auto info = item.fpsInfo.stepwise;
				std::cout << "\t\t\tFPS: " << info.minNumerator << "/" << info.minDenominator << "~" << info.maxNumerator << "/"
						  << info.maxDenominator << ", Step: " << info.stepNumerator << "/" << info.stepDenominator << std::endl;
				break;
			}
		}
	}
}

// 打印视频帧分辨率信息
static void printVideoFrameSizeInfo(size_t listSize, VideoFrameSizeInfo* list) {
	// 遍历所有分辨率
	for (size_t i = 0; i < listSize; i++) {
		// 获取分辨率信息
		auto item = list[i];
		// 区分分辨率类型
		switch (item.sizeType) {
			// 离散型
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_DISCRETE: {
				// 获取离散型分辨率信息
				auto info = item.sizeInfo.discrete;
				// 打印分辨率信息
				std::cout << "\t\tSize: " << info.width << "x" << info.height << std::endl;
				// 打印帧率信息
				printVideoFrameFpsInfo(info.fpsInfoListSize, info.fpsInfoList);
				break;
			}

			// 连续型、步进型
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_CONTINUOUS:
			case VideoFrameSizeType::VIDEO_FRAME_SIZE_TYPE_STEPWISE: {
				// 获取连续型、步进型分辨率信息
				auto info = item.sizeInfo.stepwise;
				// 打印分辨率信息
				std::cout << "\t\tSize: " << info.minWidth << "-" << info.maxWidth << "x" << info.minHeight << "-" << info.maxHeight
						  << ", Step: " << info.stepWidth << "/" << info.stepHeight << std::endl;
				// 打印帧率信息
				printVideoFrameFpsInfo(info.fpsInfoListSize, info.fpsInfoList);
				break;
			}
		}
	}
}

// 打印视频帧格式信息
static void printVoidFrameFormatInfo(size_t listSize, VideoFrameInfo* list) {
	// 遍历视频帧格式信息
	for (size_t i = 0; i < listSize; i++) {
		// 获取视频帧格式信息
		auto item = list[i];
		// 打印格式
		std::cout << "\tFormat: " << item.format << ", BufType: " << item.capType << std::endl;
		// 打印分辨率
		printVideoFrameSizeInfo(item.sizeInfoListSize, item.sizeInfoList);
	}
}
