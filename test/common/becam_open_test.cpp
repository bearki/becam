#include "custom_printf.cpp"
#include <becam/becam.h>
#include <fstream>
#include <iostream>

int main() {
	// 初始化句柄
	auto handle = BecamNew();
	if (handle == nullptr) {
		std::cerr << "Failed to initialize handle." << std::endl;
		return 1;
	}

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
	VideoFrameCaptureInfo frameInfo = {0};
	frameInfo.capType = VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_MP;
	frameInfo.format = 0x32424752;
	frameInfo.width = 1920;
	frameInfo.height = 1080;

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
		std::cout << std::endl;

		// 获取设备支持的视频帧信息
		GetDeviceConfigListReply configReply = {0};
		res = BecamGetDeviceConfigList(handle, item.devicePath, &configReply);
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
	// 释放列表
	BecamFreeDeviceList(handle, &reply);

	// 当前选中的设别路径和帧信息
	std::cout << "\n\nSelected device path: " << devicePath << std::endl;
	std::cout << "Selected frame info: " << frameInfo.width << "x" << frameInfo.height << ", " << frameInfo.capType << ", "
			  << frameInfo.numerator << "/" << frameInfo.denominator << ", " << frameInfo.format << std::endl;

	// 来个死循环
	while (true) {
		// 打开设备
		res = BecamOpenDevice(handle, devicePath.c_str(), &frameInfo);
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			std::cerr << "Failed to open device. errno: " << res << std::endl;
			return 1;
		}
		// 关闭设备
		BecamCloseDevice(handle);
	}

	// 释放句柄
	BecamFree(&handle);

	// OK
	return 0;
}