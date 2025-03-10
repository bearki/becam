#!/bin/bash

# 声明有异常时立即终止
set -e


######################## 外部变量声明 ########################
# 声明系统架构（arm、aarch64）
BuildArch=${1:-"aarch64"}
# 声明库版本号
BuildVersion=${2:-"2.0.0.0"}
# 声明工具链所在位置
Toolchain=${3:-"$HOME/build-tools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu"}
# 工具链中编译器的名称（可选：默认会自动查找）（例如：arm-linux-gnueabihf，arm-none-linux-gnueabihf，aarch64-none-linux-gnu）
ToolchainCompilerName=${4:-""}


######################## 编译器准备 ##########################
# 是否需要自动查找编译器
if [[ -z $ToolchainCompilerName ]]; then
  # 在工具链 bin 目录下查找以 "-gcc" 结尾的非递归文件
  files=$(find "${Toolchain}/bin" -maxdepth 1 -name "*-gcc")
  # 统计查询数量
  count=$(echo "$files" | wc -l)
  if [ "$count" -eq 1 ]; then
      # 提取文件名并移除结尾的 "-gcc"
      compiler=$(basename "$files")
      ToolchainCompilerName="${compiler%-gcc}"
  else
      # 多个结果时提示用户手动选择
      echo "找到多个匹配的编译器，请手动指定其中一个："
      printf "%s\n" "$files"
      exit 1
  fi
fi
# 提取GCC版本号，提取前两位版本号
tmpGccVersion=$($Toolchain/bin/$ToolchainCompilerName-gcc --version | grep -oP '\d+\.\d+\.\d+' | cut -d. -f1-2)
gccVersion="gcc-${tmpGccVersion}"

######################## 内部变量声明 ########################
# 版本号移除前置v、V
buildVersionNumber="${BuildVersion#[Vv]}"
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
echo "[系统:Linux]"
echo "[架构:${BuildArch}]"
echo "[模式:${buildType}]"
echo "[工具链:${Toolchain}]"
echo "[编译器名称:${ToolchainCompilerName}]"
echo "[编译器版本:${gccVersion}]"
    
# 执行CMake
echo "------------------------------- 执行CMake:配置 -------------------------------"
cmake -G "Unix Makefiles" \
    -DTOOLCHAIN_PATH="${Toolchain}" \
    -DTOOLCHAIN_COMPILER_NAME="${ToolchainCompilerName}" \
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
sed -i "s@ENV_LIBRARY_VERSION@${buildVersionNumber}@g" "${installDir}/libbecam_linux_${BuildArch}_v4l2/becam.pc"
# 压缩库
tar -czvf "${publishDir}/libbecam_linux_${BuildArch}_v4l2_gnu.tar.gz" -C "${installDir}/libbecam_linux_${BuildArch}_v4l2" .

# 构建结束
echo "--------------------------------- 构建:结束 ----------------------------------"
