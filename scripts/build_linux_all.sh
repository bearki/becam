#!/bin/bash

# 声明有异常时立即终止
set -e

# 编译版本号
BuildVersion="2.0.0.0"

# 脚本目录
scriptDir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# # [GCC-8.3 arm] 交叉编译
# Toolchain="$HOME/build-tools/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf"
# $scriptDir/build_linux_any.sh "arm" "$BuildVersion" "${Toolchain}"

# # [GCC-8.3 aarch64] 交叉编译
# Toolchain="$HOME/build-tools/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu"
# $scriptDir/build_linux_any.sh "aarch64" "$BuildVersion" "${Toolchain}"

# # [GCC-10.3 arm] 交叉编译
# Toolchain="$HOME/build-tools/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf"
# $scriptDir/build_linux_any.sh "arm" "$BuildVersion" "${Toolchain}"

# # [GCC-10.3 aarch64] 交叉编译
# Toolchain="$HOME/build-tools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu"
# $scriptDir/build_linux_any.sh "aarch64" "$BuildVersion" "${Toolchain}"

# [GCC-10.3 i686] 交叉编译
Toolchain="$HOME/build-tools/x86-i686--glibc--stable-2021.11-1"
$scriptDir/build_linux_any.sh "i686" "$BuildVersion" "${Toolchain}" "i686-buildroot-linux-gnu"

# [GCC-10.3 x86_64] 交叉编译
Toolchain="$HOME/build-tools/x86-64--glibc--stable-2021.11-5"
$scriptDir/build_linux_any.sh "x86_64" "$BuildVersion" "${Toolchain}" "x86_64-buildroot-linux-gnu"

# [GCC-10.3 i686] 交叉编译
# Toolchain="$HOME/build-tools/x86-i686--musl--stable-2021.11-1"
# $scriptDir/build_linux_any.sh "i686" "$BuildVersion" "${Toolchain}" "i686-buildroot-linux-musl"

# [GCC-10.3 x86_64] 交叉编译
# Toolchain="$HOME/build-tools/x86-64--musl--stable-2021.11-5"
# $scriptDir/build_linux_any.sh "x86_64" "$BuildVersion" "${Toolchain}" "x86_64-buildroot-linux-musl"
