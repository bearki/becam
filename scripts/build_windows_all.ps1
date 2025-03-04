# 定义输出编码（对[Console]::WriteLine生效）
$OutputEncoding = [console]::InputEncoding = [console]::OutputEncoding = [Text.UTF8Encoding]::UTF8
# 遇到错误立即停止
$ErrorActionPreference = 'Stop'

# 编译版本号
$BuildVersion = "2.0.0.0"
# MSVC编译
& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "i686" -BuildVersion "${BuildVersion}"  -VsVersion "Visual Studio 16 2019"
& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "x86_64" -BuildVersion "${BuildVersion}"  -VsVersion "Visual Studio 16 2019"
# 使用MinGW编译
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "i686" -BuildVersion "${BuildVersion}"  -Toolchain "${Env:MinGW32}"
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "x86_64" -BuildVersion "${BuildVersion}"  -Toolchain "${Env:MinGW64}"
