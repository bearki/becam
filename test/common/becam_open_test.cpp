#include <becam/becam.h>
#include <fstream>
#include <pkg/LogOutput.hpp>

int main() {
	// 初始化句柄
	auto handle = BecamNew();
	if (handle == nullptr) {
		DEBUG_LOG("Failed to initialize handle.");
		return 1;
	}

	// 声明返回值
	GetDeviceListReply reply;
	// 获取设备列表
	auto res = BecamGetDeviceList(handle, &reply);
	if (res != StatusCode::STATUS_CODE_SUCCESS) {
		DEBUG_LOG("Failed to get device list. errno: " << res);
		BecamFree(&handle);
		return 1;
	}

	// 选中的设备路径
	std::string devicePath = "";
	// 选中的视频帧信息
	VideoFrameInfo frameInfo = {0};

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
			DEBUG_LOG("Failed to get device config list. errno: " << res);
			// 释放列表
			BecamFreeDeviceList(&reply);
			BecamFree(&handle);
			return 1;
		}

		// 遍历视频帧信息
		for (size_t j = 0; j < configReply.videoFrameInfoListSize; j++) {
			// 获取帧信息
			auto element = configReply.videoFrameInfoList[j];
			// 打印一下
			std::cout << "\t"
					  << "width: " << element.width << ", "
					  << "height: " << element.height << ", "
					  << "fps: " << element.fps << ", "
					  << "format: " << element.format << std::endl;
			// 提取一个帧信息
			if (devicePath.empty()) {
				devicePath = item.devicePath;
				frameInfo = element;
			}
		}
		// 释放支持的配置列表
		BecamFreeDeviceConfigList(&configReply);
	}
	// 释放列表
	BecamFreeDeviceList(&reply);

	// 当前选中的设别路径和帧信息
	std::cout << "\n\nSelected device path: " << devicePath << std::endl;
	std::cout << "Selected frame info: " << frameInfo.width << "x" << frameInfo.height << ", " << frameInfo.fps << ", " << frameInfo.format
			  << std::endl;

	// 来个死循环
	while (true) {
		// 打开设备
		res = BecamOpenDevice(handle, devicePath.c_str(), &frameInfo);
		if (res != StatusCode::STATUS_CODE_SUCCESS) {
			DEBUG_LOG("Failed to open device. errno: " << res);
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