#include <becam/becam.h>
#include <fstream>
#include <iostream>
#include "custom_printf.hpp"



int main() {
	// 初始化句柄
	auto handle = BecamNew();
	if (handle == nullptr) {
		std::cerr << "Failed to initialize handle." << std::endl;
		return 1;
	}

	// 来个死循环
	// while (true) {
	// 声明返回值
	GetDeviceListReply reply;
	// 获取设备列表
	auto res = BecamGetDeviceList(handle, &reply);
	if (res != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "Failed to get device list. errno: " << res << std::endl;
		BecamFree(&handle);
		return 1;
	}

	// 打印一下设备列表
	for (size_t i = 0; i < reply.deviceInfoListSize; i++) {
		// 获取设备信息
		auto item0 = reply.deviceInfoList[i];
		if (item0.name) {
			std::cout << "\n\nName: " << item0.name;
		}
		if (item0.devicePath) {
			std::cout << "\nDevicePath: " << item0.devicePath;
		}
		std::cout << std::endl;

		// 获取设备支持的视频帧信息
		GetDeviceConfigListReply configReply = {0};
		res = BecamGetDeviceConfigList(handle, item0.devicePath, &configReply);
		std::cout << "BecamGetDeviceConfigList res=" << res << ", resSize=" << configReply.videoFrameInfoListSize << std::endl;
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			std::cerr << "Failed to get device config list. errno: " << res << std::endl;
			// 释放列表
			BecamFreeDeviceList(handle, &reply);
			BecamFree(&handle);
			return 1;
		}

		// 打印视频帧格式信息
		printVoidFrameFormatInfo(configReply.videoFrameInfoListSize, configReply.videoFrameInfoList);
		// 释放支持的配置列表
		BecamFreeDeviceConfigList(handle, &configReply);
	}
	// 释放设备列表
	BecamFreeDeviceList(handle, &reply);
	// }
	// 释放句柄
	BecamFree(&handle);

	// OK
	return 0;
}