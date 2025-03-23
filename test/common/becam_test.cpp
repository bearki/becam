#include <cli11/include/CLI/CLI.hpp>
#include "becam_devices_test.cpp"

int main(int argc, char* argv[]) {
	// 创建解析器
	CLI::App app{"Becam Library Test Tool"};
	// 启用UTF8保证
	argv = app.ensure_utf8(argv);

	// 添加获取设备列表子命令
	CLI::App* getDevices = app.add_subcommand("devices", "枚举设备列表");
    // 为子命令设置回调
	getDevices->callback(enumDevices);

	// 添加获取分辨率列表子命令
	CLI::App* getConfigs = app.add_subcommand("configs", "枚举分辨率列表");
	// 指定设备路径选项
	std::string devicePath;
	getConfigs->add_option("device", devicePath, "设备路径")->required();

	// 添加取流子命令
	CLI::App* getFrame = app.add_subcommand("stream", "图像流采集");

	// 执行参数解析
	CLI11_PARSE(app, argc, argv);
	// OK
	return 0;
}