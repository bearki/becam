#!/bin/bash

# 声明有异常时立即终止
set -e

# 编译版本号
BuildVersion="2.0.0.0"

# 脚本目录
scriptDir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# 编译
Toolchain="$HOME/build-tools/android-ndk-r26d"
# 32位
$scriptDir/build_android_any.sh "armeabi-v7a" "$BuildVersion" "${Toolchain}"
# 64位
$scriptDir/build_android_any.sh "arm64-v8a" "$BuildVersion" "${Toolchain}"
