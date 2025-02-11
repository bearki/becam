#!/bin/bash

# 声明有异常时立即终止
set -e


######################## 外部变量声明 ########################
# 声明使用的平台（rk1126、rk1109、rk3566、rk3588）
Platform=${1:-"rk1126"}
# 声明系统架构（arm、aarch64）
BuildArch=${2:-"arm"}
# 声明工具链所在位置
Toolchain=${3:-"$HOME/build-tools/RV1126_RV1109_LINUX_SDK_V2.2.4/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf"}


######################## 内部变量声明 ########################
# 项目目录
projectDir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# 构建目录
buildDir="${projectDir}/build"
# 安装目录
installDir="${projectDir}/install/${Platform}"
# 发布目录
publishDir="${projectDir}/publish"
# 编译类型（Debug、Release）
buildType="Release"

######################### 环境准备 ##########################
# 移除旧的构建目录
rm -rf "${buildDir}"
# 创建新的构建目录
mkdir -p -m 755 "${buildDir}"
if [[ ! -d "${installDir}" ]]; then
  mkdir -p -m 755 "${installDir}"
fi
if [[ ! -d "${publishDir}" ]]; then
  mkdir -p -m 755 "${publishDir}"
fi

# 构建开始
echo "--------------------------------- 构建:开始 ----------------------------------"
echo "[平台:${Platform}]"
echo "[系统:Linux]"
echo "[架构:${BuildArch}]"
echo "[模式:${buildType}]"
echo "[工具链:${Toolchain}]"
    
# 执行CMake
echo "------------------------------- 执行CMake:配置 -------------------------------"
cmake -G "Unix Makefiles" \
    -DTOOLCHAIN_PATH="${Toolchain}" \
    -DCMAKE_SYSTEM_PROCESSOR="${BuildArch}" \
    -DCMAKE_TOOLCHAIN_FILE="${projectDir}/cmake-toolchains/linux-rockchip-toolchain.cmake" \
    -DCMAKE_BUILD_TYPE="${buildType}" \
    -S "${projectDir}" \
    -B "${buildDir}"


# 执行make
echo "------------------------------- 执行CMake:构建 -------------------------------"
cmake --build "${buildDir}" --config "${buildType}"

# 执行make install
echo "------------------------------- 执行CMake:安装 -------------------------------"
cmake --install "${buildDir}" --config "${buildType}" --prefix "${installDir}"

# 执行压缩
echo "---------------------------------- 执行压缩 ----------------------------------"
tar -czvf "${publishDir}/libbecamv4l2_linux_${BuildArch}_${Platform}.tar.gz" -C "${installDir}/libbecamv4l2_linux_${BuildArch}" .

# 构建结束
echo "--------------------------------- 构建:结束 ----------------------------------"
