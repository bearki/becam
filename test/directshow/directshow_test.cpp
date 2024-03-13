#include <dshow.h>
#include <windows.h>

#include <directshow/directshow.cpp>
#include <iostream>

int main() {
    // 声明返回值
    GetDeviceListReply reply;
    // 获取设备列表
    auto res = GetDeviceList(&reply);
    if (res != StatusCode::STATUS_CODE_SUCCESS) {
        std::cerr << "Failed to get device list. errno: " << res << std::endl;
        return 1;
    }

    // OK
    return 0;
}