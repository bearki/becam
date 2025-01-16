#!/bin/bash

# 声明有异常时立即终止
set -e

# 脚本目录
scriptDir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# x86编译
"${scriptDir}/build_linux_gnu_any.sh" "i686"
"${scriptDir}/build_linux_gnu_any.sh" "x86_64"
# Rockchip编译
RK1126Toolchain="/home/WORK/Downloads/RV1126_RV1109_LINUX_SDK_V2.2.4/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf"
"${scriptDir}/build_linux_rockchip_any.sh" "rk1126" "arm" "${RK1126Toolchain}"
