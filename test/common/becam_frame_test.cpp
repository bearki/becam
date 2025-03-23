#include "custom_printf.hpp"
#include <becam/becam.h>
#include <fstream>
#include <iostream>
#include <linux/videodev2.h>

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
	std::string devicePath = "/dev/video11";
	// 选中的视频帧信息
	VideoFrameCaptureInfo capInfo = {0};
	capInfo.capType = VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_MP;
	capInfo.format = V4L2_PIX_FMT_NV12;
	capInfo.width = 1280;
	capInfo.height = 720;
	capInfo.numerator = 0;
	capInfo.denominator = 0;

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
	// 释放设备列表
	BecamFreeDeviceList(handle, &reply);

	// 当前选中的设别路径和帧信息
	std::cout << "\n\nSelected device path: " << devicePath << std::endl;
	std::cout << "Selected frame info: " << capInfo.width << "x" << capInfo.height << ", " << capInfo.capType << ", " << capInfo.numerator
			  << "/" << capInfo.denominator << ", " << capInfo.format << std::endl;

	// 打开设备
	res = BecamOpenDevice(handle, devicePath.c_str(), &capInfo);
	if (res != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "Failed to open device. errno: " << res << std::endl;
		BecamFree(&handle);
		return 1;
	}

	// 来个死循环
	while (true) {
		// 获取一帧
		uint8_t* data = nullptr;
		size_t size = 0;
		res = BecamGetFrame(handle, &data, &size);
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			std::cout << "Frame empty. 000000000000000000000000000000000000000000000000000, Code:" << res << std::endl;
			continue;
		} else {
			std::cout << "OK, Write Size: " << size << std::endl;
		}

		// 数据写入到文件
		// 打开或创建一个文件以二进制模式写入
		std::ofstream ofs("output.jpg", std::ios::binary | std::ios::out);
		// 检查文件是否成功打开
		if (ofs.is_open()) {
			// 将字节数组写入文件
			ofs.write((char*)data, static_cast<std::streamsize>(size));
			// 检查是否所有数据都已成功写入
			if (!ofs) {
				std::cerr << "Error writing to file!" << std::endl;
			}
			// 关闭文件流
			ofs.close();
		} else {
			std::cerr << "Unable to open file for writing." << std::endl;
		}
		// 释放帧
		BecamFreeFrame(handle, &data);
	}

	// 关闭设备
	BecamCloseDevice(handle);
	// 释放句柄
	BecamFree(&handle);
	// OK
	return 0;
}