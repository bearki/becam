#include "Becamv4l2DeviceHelper.hpp"
#include "Becamv4l2DeviceConfigHelper.hpp"
#include "xioctl.hpp"
#include <algorithm>
#include <fcntl.h>
#include <glob.h>
#include <iostream>
#include <linux/videodev2.h>
#include <pkg/StringConvert.hpp>
#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

/**
 * @implements 实现构造函数
 */
Becamv4l2DeviceHelper::Becamv4l2DeviceHelper() {}

/**
 * @implements 实现析构函数
 */
Becamv4l2DeviceHelper::~Becamv4l2DeviceHelper() {
	// 释放当前设备
	this->closeCurrentDevice();
}

/**
 * @implements 实现关闭当前设备
 */
void Becamv4l2DeviceHelper::closeCurrentDevice() {
	// 停止当前设备取流
	this->stopCurrentDeviceStreaming();
	// 已打开的设备需要关闭设备
	if (this->activatedDevice != -1) {
		// 关闭设备
		close(this->activatedDevice);
		this->activatedDevice = -1;
	}
}

/**
 * @implements 实现停止当前设备取流
 */
void Becamv4l2DeviceHelper::stopCurrentDeviceStreaming() {
	// 正在取流的需要先停止
	if (this->streamON) {
		// 停止取流
		auto bufType = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE;
		xioctl(this->activatedDevice, VIDIOC_STREAMOFF, &bufType);
		// 标记已停止取流
		this->streamON = false;
	}
	// 取消内核缓冲区和用户缓冲区的映射
	for (size_t i = 0; i < Becamv4l2DeviceHelper::USER_BUFFER_COUNT; i++) {
		// 仅在缓冲区长度有效时进行解绑
		if (this->userBufferLengths[i] > 0) {
			munmap(this->userBuffers[i], this->userBufferLengths[i]);
			this->userBuffers[i] = nullptr;
			this->userBufferLengths[i] = 0;
		}
	}
}

/**
 * @implements 实现处理设备名称
 */
std::string Becamv4l2DeviceHelper::trimDeviceName(const std::string& deviceName) {
	// 预声明
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
 * @implements 实现检查设备是否支持视频捕获能力
 */
bool Becamv4l2DeviceHelper::isVideoCaptureDevice(const std::string& devicePath, std::string& deviceName) {
	// 只读方式打开设备句柄
	auto fd = open(devicePath.c_str(), O_RDONLY);
	if (fd == -1) {
		std::cerr << "Becamv4l2DeviceHelper::isVideoCaptureDevice -> open(" << devicePath << ") Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return false;
	}

	// 使用xioctl查询设备是否支持视频捕获能力
	v4l2_capability cap;
	auto res = xioctl(fd, VIDIOC_QUERYCAP, &cap);
	// 关闭句柄
	close(fd);
	// 检查结果
	if (res == -1) {
		std::cerr << "Becamv4l2DeviceHelper::isVideoCaptureDevice -> xioctl(VIDIOC_QUERYCAP) Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return false;
	}

	// 检查能力
	if (cap.capabilities & V4L2_CAP_DEVICE_CAPS // 设备总体能力必须支持Device Capabilities，表示 device_caps 字段有效
		&& (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE ||
			cap.device_caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE) // 当前设备节点访问的能力必须支持视频捕获（单平面、多平面都可以）
	) {
		// 复制设备名称
		deviceName = Becamv4l2DeviceHelper::trimDeviceName(reinterpret_cast<char*>(cap.card));
		return true;
	}

	// 默认无能力
	return false;
}

/**
 * @implements 实现获取设备列表
 */
StatusCode Becamv4l2DeviceHelper::GetDeviceList(DeviceInfo*& reply, size_t& replySize) {
	// 重置
	reply = nullptr;
	replySize = 0;

	// 查找符合`/dev/video*`的设备
	glob_t globResult;
	auto res = glob("/dev/video*", GLOB_TILDE, nullptr, &globResult);
	if (res != 0) {
		// 无匹配时直接返回
		if (res == GLOB_NOMATCH) {
			return StatusCode::STATUS_CODE_SUCCESS;
		}
		std::cerr << "Becamv4l2DeviceHelper::GetDeviceList -> lob(/dev/video*, GLOB_TILDE) Failed, RESULT:" << res << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_ENUM_FAILED;
	}

	// 临时设备列表
	std::vector<DeviceInfo> deviceList;
	// 遍历匹配结果
	for (size_t i = 0; i < globResult.gl_pathc; i++) {
		// 待提取的设备名称
		std::string deviceName = "";
		// 提取设备路径
		std::string devicePath = globResult.gl_pathv[i];
		if (Becamv4l2DeviceHelper::isVideoCaptureDevice(devicePath, deviceName)) {
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

/**
 * @implements 实现激活指定设备
 */
StatusCode Becamv4l2DeviceHelper::ActivateDevice(const std::string& devicePath, const int oflags) {
	// 参数检查
	if (devicePath.empty()) {
		return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
	}

	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 释放已激活的设备
	this->closeCurrentDevice();

	// 检查设备是否存在
	if (access(devicePath.c_str(), F_OK) != 0) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_FOUND;
	}

	// 激活设备
	this->activatedDevice = open(devicePath.c_str(), oflags);
	if (this->activatedDevice == -1) {
		std::cerr << "Becamv4l2DeviceHelper::ActivateDevice -> open(" << devicePath << ") Failed" << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_OPEN_FAILED;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现获取当前设备支持的配置列表
 */
StatusCode Becamv4l2DeviceHelper::GetCurrentDeviceConfigList(VideoFrameInfo*& reply, size_t& replySize) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已激活
	if (this->activatedDevice == -1) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_OPEN;
	}

	// 初始化设备配置助手类
	auto configHelper = Becamv4l2DeviceConfigHelper(this->activatedDevice);
	// 查询设备支持的配置列表
	return configHelper.GetDeviceConfigList(reply, replySize);
}

/**
 * @implements 实现释放已获取的设备支持的配置列表
 */
void Becamv4l2DeviceHelper::FreeDeviceConfigList(VideoFrameInfo*& input, size_t& inputSize) {
	// 执行释放
	Becamv4l2DeviceConfigHelper::FreeDeviceConfigList(input, inputSize);
}

/**
 * @implements 实现激活设备取流
 */
StatusCode Becamv4l2DeviceHelper::ActivateDeviceStreaming(const VideoFrameCaptureInfo& capInfo) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 检查设备是否已激活
	if (this->activatedDevice == -1) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_OPEN;
	}

	// 停止当前设备取流
	this->stopCurrentDeviceStreaming();

	// 声明输出格式和分辨率
	v4l2_format format = {0};
	// 区别捕获类型
	switch (capInfo.capType) {
		// 单平面捕获
		case VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_SP: {
			format.type = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE; // 指定捕获类型
			format.fmt.pix.pixelformat = capInfo.format;			  // 指定帧格式
			format.fmt.pix.width = capInfo.width;					  // 指定帧分辨率
			format.fmt.pix.height = capInfo.height;					  // 指定帧分辨率
			format.fmt.pix.field = v4l2_field::V4L2_FIELD_NONE;		  // 指定场模式，通常为：V4L2_FIELD_NONE
			break;
		}

		// 多平面捕获
		case VideoFrameCaptureType::VIDEO_FRAME_CAPTURE_TYPE_MP: {
			format.type = v4l2_buf_type::V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE; // 指定捕获类型
			format.fmt.pix_mp.pixelformat = capInfo.format;					 // 指定帧格式
			format.fmt.pix_mp.width = capInfo.width;						 // 指定帧分辨率
			format.fmt.pix_mp.height = capInfo.height;						 // 指定帧分辨率
			format.fmt.pix_mp.field = v4l2_field::V4L2_FIELD_NONE;			 // 指定场模式
			break;
		}

		// 默认不支持
		default: {
			std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> Unsupported capture types " << capInfo.capType << std::endl;
			return StatusCode::STATUS_CODE_ERR_INPUT_PARAM;
		}
	}
	// 设置分辨率和格式
	if (xioctl(this->activatedDevice, VIDIOC_S_FMT, &format) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> xioctl(VIDIOC_S_FMT) Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_FRAME_FMT_SET_FAILED;
	}

	// 声明输出帧率
	v4l2_streamparm streamparm = {0};
	streamparm.type = format.type;											// 设置捕获类型
	streamparm.parm.capture.timeperframe.numerator = capInfo.numerator;		// 帧率分子
	streamparm.parm.capture.timeperframe.denominator = capInfo.denominator; // 帧率分母
	// 设置输出帧率
	if (xioctl(this->activatedDevice, VIDIOC_S_PARM, &streamparm) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> xioctl(VIDIOC_S_PARM) Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_FRAME_FMT_SET_FAILED;
	}

	// 请求缓冲区
	v4l2_requestbuffers reqBuf = {0};
	reqBuf.count = Becamv4l2DeviceHelper::USER_BUFFER_COUNT;
	reqBuf.type = format.type;
	reqBuf.memory = v4l2_memory::V4L2_MEMORY_MMAP;
	if (xioctl(this->activatedDevice, VIDIOC_REQBUFS, &reqBuf) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> xioctl(VIDIOC_REQBUFS) Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return StatusCode::STATUS_CODE_V4L2_ERR_REQUEST_BUF;
	}

	// 声明内核缓冲区查询参数
	v4l2_buffer buf = {0};
	// 查询内核缓冲区，并将其映射到用户缓冲区
	for (int i = 0; i < Becamv4l2DeviceHelper::USER_BUFFER_COUNT; i++) {
		// 查询内核缓冲区
		buf.type = format.type;
		buf.memory = v4l2_memory::V4L2_MEMORY_MMAP;
		buf.index = i;
		if (xioctl(this->activatedDevice, VIDIOC_QUERYBUF, &buf) == -1) {
			std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> xioctl(VIDIOC_QUERYBUF) Failed, " << strerror(errno)
					  << ", errno: " << errno << std::endl;
			return StatusCode::STATUS_CODE_V4L2_ERR_QUERY_BUF;
		}
		// 映射内核缓冲区
		this->userBuffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, this->activatedDevice, buf.m.offset);
		if (this->userBuffers[i] == MAP_FAILED) {
			std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceStreaming -> xioctl(VIDIOC_QUERYBUF) Failed, " << strerror(errno)
					  << ", errno: " << errno << std::endl;
			return StatusCode::STATUS_CODE_V4L2_ERR_MMAP_BUF;
		}
		// 储存缓冲区的长度
		this->userBufferLengths[i] = buf.length;
	}

	// 将缓冲区加入到设备的输出队列（就是缓冲区解锁的意思）
	for (int i = 0; i < Becamv4l2DeviceHelper::USER_BUFFER_COUNT; i++) {
		buf.type = format.type;
		buf.memory = v4l2_memory::V4L2_MEMORY_MMAP;
		buf.index = i;
		if (xioctl(this->activatedDevice, VIDIOC_QBUF, &buf) == -1) {
			std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceRender -> xioctl(VIDIOC_QBUF) Failed, " << strerror(errno)
					  << ", errno: " << errno << std::endl;
			return StatusCode::STATUS_CODE_V4L2_ERR_UNLOCK_BUF;
		}
	}

	// 启动视频流
	auto bufType = format.type;
	if (xioctl(this->activatedDevice, VIDIOC_STREAMON, &bufType) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::ActivateDeviceRender -> xioctl(VIDIOC_STREAMON) Failed, " << strerror(errno)
				  << ", errno: " << errno << std::endl;
		return StatusCode::STATUS_CODE_ERR_DEVICE_RUN_FAILED;
	}
	// 标记已经开始取流
	this->streamON = true;

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现关闭设备
 */
void Becamv4l2DeviceHelper::CloseDevice() {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 关闭当前设备
	this->closeCurrentDevice();
}

/**
 * @implements 实现获取视频帧
 */
StatusCode Becamv4l2DeviceHelper::GetFrame(uint8_t*& reply, size_t& replySize) {
	// 加个锁先
	std::unique_lock<std::mutex> lock(this->mtx);

	// 重置
	reply = nullptr;
	replySize = 0;

	// 检查设备是否已激活
	if (this->activatedDevice == -1) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_OPEN;
	}
	if (!this->streamON) {
		return StatusCode::STATUS_CODE_ERR_DEVICE_NOT_RUN;
	}

	// 声明缓冲区队列查询参数
	v4l2_buffer buf = {0};
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	// 消费队列中的缓冲区（就是缓冲区加锁）
	if (xioctl(this->activatedDevice, VIDIOC_DQBUF, &buf) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::GetFrame -> xioctl(VIDIOC_DQBUF) Failed, " << strerror(errno) << ", errno: " << errno
				  << std::endl;
		return StatusCode::STATUS_CODE_V4L2_ERR_LOCK_BUF;
	}

	// 是否读取到有效帧
	if (buf.bytesused > 0) {
		// 拷贝帧
		replySize = buf.bytesused;
		reply = new uint8_t[replySize];
		memcpy(reply, this->userBuffers[buf.index], replySize);
	}

	// 重新将缓冲区加入队列（就是缓冲区解锁）
	if (xioctl(this->activatedDevice, VIDIOC_QBUF, &buf) == -1) {
		std::cerr << "Becamv4l2DeviceHelper::GetFrame -> xioctl(VIDIOC_QBUF) Failed, " << strerror(errno) << ", errno: " << errno
				  << std::endl;
		return StatusCode::STATUS_CODE_V4L2_ERR_UNLOCK_BUF;
	}

	// 检查视频帧是否无效
	if (replySize <= 0) {
		return StatusCode::STATUS_CODE_ERR_GET_FRAME_EMPTY;
	}

	// OK
	return StatusCode::STATUS_CODE_SUCCESS;
}

/**
 * @implements 实现释放已获取的视频帧
 */
void Becamv4l2DeviceHelper::FreeFrame(uint8_t*& reply) {
	if (reply == nullptr) {
		return;
	}

	delete[] reply;
	reply = nullptr;
}
