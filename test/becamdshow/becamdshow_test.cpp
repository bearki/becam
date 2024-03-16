#include <becam/becam.h>
#include <iostream>

int main() {
	// 是否进行内存测试
	bool memoryTest = true;

	// 来个死循环
	do {
		// 初始化句柄
		auto handle = BecamNew();
		// 声明返回值
		GetDeviceListReply reply;
		// 获取设备列表
		auto res = BecamGetDeviceList(handle, &reply);
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			std::cerr << "Failed to get device list. errno: " << res << std::endl;
			BecamFree(&handle);
			return 1;
		}

		// 选中的设备路径
		std::string devicePath = "";
		// 选中的视频帧信息
		VideoFrameInfo* frameInfo = nullptr;

		// 打印一下设备列表
		for (size_t i = 0; i < reply.deviceInfoListSize; i++) {
			// 获取设备信息
			auto item = reply.deviceInfoList[i];
			if (item.name) {
				std::cout << "\n\nName: " << item.name;
			}
			if (item.devicePath) {
				std::cout << "\nDevicePath: " << item.devicePath;
			}
			if (item.locationInfo) {
				std::cout << "\nLocationInfo: " << item.locationInfo;
			}
			std::cout << std::endl;
			// 遍历视频帧信息
			for (size_t j = 0; j < item.frameInfoListSize; j++) {
				// 获取帧信息
				auto element = item.frameInfoList[j];
				// 打印一下
				std::cout << "\t"
						  << "width: " << element.width << ", "
						  << "height: " << element.height << ", "
						  << "fps: " << element.fps << ", "
						  << "format: " << element.format << std::endl;
				// 提取一个帧信息
				if (frameInfo == nullptr) {
					devicePath = item.devicePath;
					frameInfo = new VideoFrameInfo();
					*frameInfo = element;
				}
			}
		}

		// 释放列表
		BecamFreeDeviceList(handle, &reply);

		// 当前选中的设别路径和帧信息
		std::cout << "\n\nSelected device path: " << devicePath << std::endl;
		if (frameInfo == nullptr) {
			std::cout << "Selected frame info: nullptr" << std::endl;
		} else {
			std::cout << "Selected frame info: " << frameInfo->width << "x" << frameInfo->height << ", "
					  << frameInfo->fps << ", " << frameInfo->format << std::endl;

			// 打开设备
			res = BecamOpenDevice(handle, devicePath.c_str(), frameInfo);
			if (res != StatusCode::STATUS_CODE_SUCCESS) {
				std::cerr << "Failed to open device. errno: " << res << std::endl;
			} else {
				// 获取100帧
				for (size_t i = 0; i < 100; i++) {
					uint8_t* data = nullptr;
					size_t size = 0;
					BecamGetFrame(handle, &data, &size);
					BecamFreeFrame(handle, &data);
				}
			}

			// 释放帧信息
			delete frameInfo;
		}

		// 释放句柄
		BecamFree(&handle);
	} while (memoryTest);

	// OK
	return 0;
}