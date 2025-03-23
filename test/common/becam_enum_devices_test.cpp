#include <becam/becam.h>
#include <iostream>

// 枚举设备列表
void enumDevices() {
	// 初始化句柄
	auto handle = BecamNew();
	if (handle == nullptr) {
		std::cerr << "Failed to initialize handle." << std::endl;
		return;
	}

	// 声明返回值
	GetDeviceListReply reply;
	// 获取设备列表
	auto res = BecamGetDeviceList(handle, &reply);
	if (res != StatusCode::STATUS_CODE_SUCCESS) {
		std::cerr << "Failed to get device list. errno: " << res << std::endl;
		BecamFree(&handle);
		return;
	}

	// 打印一下设备列表
	for (size_t i = 0; i < reply.deviceInfoListSize; i++) {
		// 获取设备信息
		auto item = reply.deviceInfoList[i];
		if (item.name) {
			std::cout << "\n" << i + 1 << ". Name: " << item.name;
		}
		if (item.devicePath) {
			std::cout << "\n  DevicePath: " << item.devicePath;
		}
		std::cout << std::endl;
	}

	// 释放设备列表
	BecamFreeDeviceList(handle, &reply);
	// 释放句柄
	BecamFree(&handle);
}
