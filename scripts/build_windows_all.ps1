# 编译版本号
$BuildVersion = "2.0.0.0"
# MSVC编译
& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "i686" -BuildVersion "${BuildVersion}"  -VsVersion "Visual Studio 2022"
& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "x86_64" -BuildVersion "${BuildVersion}"  -VsVersion "Visual Studio 2022"
# 使用MinGW编译
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "i686" -BuildVersion "${BuildVersion}"  -Toolchain "${Env:MinGW32}"
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "x86_64" -BuildVersion "${BuildVersion}"  -Toolchain "${Env:MinGW64}"
