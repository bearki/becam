#include "Becamv4l2DeviceHelper.hpp"
#include <algorithm>
#include <fcntl.h>
#include <glob.h>
#include <iostream>
#include <linux/videodev2.h>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include <pkg/StringConvert.hpp>

/**
 * @implements 实现构造函数
 */
Becamv4l2DeviceHelper::Becamv4l2DeviceHelper() {}

/**
 * @implements 实现析构函数
 */
Becamv4l2DeviceHelper::~Becamv4l2DeviceHelper() {}

/**
 * @brief 处理设备名称
 *
 * @param deviceName [in] 设备名称
 * @return 处理后的设备名称
 */
std::string trimDeviceName(const std::string& deviceName) {
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(deviceName);

	// 使用std::getline按冒号分割字符串
	while (std::getline(ss, token, ':')) {
		tokens.push_back(TrimSpace(token));
	}

	// 如果字符串中没有冒号，返回整个字符串
	if (tokens.empty()) {
		return deviceName;
	}

	// 找到最长的子串
	auto longest =
		std::max_element(tokens.begin(), tokens.end(), [](const std::string& a, const std::string& b) { return a.size() < b.size(); });

	// 如果没有冒号，返回原字符串
	return longest[0];
}

/**
 * @brief 检查设备是否支持视频捕获能力
 *
 * @param devicePath [in] 设备路径
 * @param deviceName [out] 设备名称（仅在设备支持视频捕获能力时返回）
 * @return 是否支持视频捕获能力
 */
static bool isVideoCaptureDevice(const std::string& devicePath, std::string& deviceName) {
	// 只读方式打开设备句柄
	auto fd = open(devicePath.c_str(), O_RDONLY);
	if (fd == -1) {
		std::cerr << "isVideoCaptureDevice(" << devicePath << ") Failed" << std::endl;
		return false;
	}

	// 使用ioctl查询设备是否支持视频捕获能力
	v4l2_capability cap;
	auto res = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	// 关闭句柄
	close(fd);
	// 检查结果
	if (res == -1) {
		return false;
	}

	// 检查能力
	if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE	// 设备总体能力必须支持视频捕获
		&& cap.capabilities & V4L2_CAP_DEVICE_CAPS	// 设备总体能力必须支持Device Capabilities，表示 device_caps 字段有效
		&& cap.device_caps & V4L2_CAP_VIDEO_CAPTURE // 当前设备节点访问的能力必须支持视频捕获
	) {
		// 复制设备名称
		deviceName = trimDeviceName(reinterpret_cast<char*>(cap.card));
		return true;
	}

	// 默认无能力
	return false;
}

/**
 * @implements 实现获取设备列表
 */
StatusCode Becamv4l2DeviceHelper::GetDeviceList(DeviceInfo*& reply, size_t& replySize) {
	// 查找符合`/dev/video*`的设备
	glob_t globResult;
	auto res = glob("/dev/video*", GLOB_TILDE, nullptr, &globResult);
	if (res != 0) {
		std::cerr << "Becamv4l2DeviceHelper::GetDeviceList -> lob(/dev/video*, GLOB_TILDE) Failed, RESULT:" << res << std::endl;
		return StatusCode::STATUS_CODE_MF_ERR_DEVICE_GLOB_MATCH;
	}

	// 临时设备列表
	std::vector<DeviceInfo> deviceList;
	// 遍历匹配结果
	for (size_t i = 0; i < globResult.gl_pathc; i++) {
		// 待提取的设备名称
		std::string deviceName = "";
		// 提取设备路径
		std::string devicePath = globResult.gl_pathv[i];
		if (isVideoCaptureDevice(devicePath, deviceName)) {
			// 构建设备信息
			DeviceInfo deviceInfo = {0};
			// 拷贝设备名称
			deviceInfo.name = new char[deviceName.length() + 1];
			memcpy(deviceInfo.name, deviceName.c_str(), deviceName.length() + 1);
			// 拷贝设备路径
			deviceInfo.devicePath = new char[devicePath.length() + 1];
			memcpy(deviceInfo.devicePath, devicePath.c_str(), devicePath.length() + 1);
			// 添加到临时列表
			deviceList.push_back(deviceInfo);
		}
	}

	// 释放泛匹配结果
	globfree(&globResult);

	// 是否需要赋值响应数据
	if (deviceList.size() > 0) {
		// 拷贝设备列表
		replySize = deviceList.size();
		reply = new DeviceInfo[replySize];
		memcpy(reply, deviceList.data(), replySize * sizeof(DeviceInfo));
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放设备列表
 */
void Becamv4l2DeviceHelper::FreeDeviceList(DeviceInfo*& input, size_t& inputSize) {
	// 检查
	if (inputSize <= 0 || input == nullptr) {
		return;
	}

	// 遍历，执行释放操作
	for (size_t i = 0; i < inputSize; i++) {
		// 获取引用
		auto item = input[i];
		// 释放友好名称
		if (item.name != nullptr) {
			delete[] item.name;
			item.name = nullptr;
		}
		// 释放设备路径
		if (item.devicePath != nullptr) {
			delete[] item.devicePath;
			item.devicePath = nullptr;
		}
	}

	// 释放整个列表
	delete[] input;
	inputSize = 0;
	input = nullptr;
}
