#!/bin/bash

# 声明有异常时立即终止
set -e


######################## 外部变量声明 ########################
# 声明系统架构（i686、x86_64）
BuildArch=${1:-"x86_64"}
# 声明库版本号
BuildVersion=${2:-"2.0.0.0"}


######################## 内部变量声明 ########################
# 项目目录
projectDir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# 构建目录
buildDir="${projectDir}/build"
# 安装目录
installDir="${projectDir}/install/gnu"
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
echo "[平台:gnu]"
echo "[系统:Linux]"
echo "[架构:${BuildArch}]"
echo "[模式:${buildType}]"
    
# 执行CMake
echo "------------------------------- 执行CMake:配置 -------------------------------"
cmake -G "Unix Makefiles" \
    -DCMAKE_SYSTEM_PROCESSOR="${BuildArch}" \
    -DCMAKE_TOOLCHAIN_FILE="${projectDir}/cmake-toolchains/linux-gnu-toolchain.cmake" \
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
sed -i "s@ENV_LIBRARY_VERSION@${BuildVersion}@g" "${installDir}/libbecam_linux_${BuildArch}_v4l2/becam.pc"
# 压缩库
tar -czvf "${publishDir}/libbecam_linux_${BuildArch}_v4l2_gnu.tar.gz" -C "${installDir}/libbecam_linux_${BuildArch}_v4l2" .

# 构建结束
echo "--------------------------------- 构建:结束 ----------------------------------"
