#!/bin/bash

# 声明有异常时立即终止
set -e


######################## 外部变量声明 ########################
# 声明系统架构（arm64-v8a、armeabi-v7a、armeabi-v6、armeabi、mips、mips64、x86、x86_64）
BuildArch=${1:-"arm64-v8a"}
# 声明库版本号
BuildVersion=${2:-"2.0.0.0"}
# 声明工具链所在位置
Toolchain=${3:-"$HOME/build-tools/android-ndk-r26d"}


######################## 内部变量声明 ########################
# 版本号移除前置v、V
buildVersionNumber="${BuildVersion//^[Vv]/}"
# 项目目录
projectDir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# 构建目录
buildDir="${projectDir}/build"
# 安装目录
installDir="${projectDir}/install/android"
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
echo "[系统:Android]"
echo "[架构:${BuildArch}]"
echo "[模式:${buildType}]"
echo "[工具链:${Toolchain}]"
    
# 执行CMake
echo "------------------------------- 执行CMake:配置 -------------------------------"
cmake -G "Unix Makefiles" \
    -DANDROID_ABI="${BuildArch}" \
    -DANDROID_PLATFORM="android-21" \
    -DANDROID_NDK="${TOOLCHAIN_PATH}" \
    -DCMAKE_TOOLCHAIN_FILE="${Toolchain}/build/cmake/android.toolchain.cmake" \
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
# 赋值pkg-config配置信息版本号
sed -i "s@ENV_LIBRARY_VERSION@${buildVersionNumber}@g" "${installDir}/libbecam_linux_${BuildArch}_uvc/becam.pc"
# 压缩库
tar -czvf "${publishDir}/libbecam_android_${BuildArch}_uvc.tar.gz" -C "${installDir}/libbecam_android_${BuildArch}_uvc" .

# 构建结束
echo "--------------------------------- 构建:结束 ----------------------------------"
