& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "i686" -VsVersion "Visual Studio 2022"
& "${PSScriptRoot}\build_windows_msvc_any.ps1" -BuildArch "x86_64" -VsVersion "Visual Studio 2022"
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "i686" -Toolchain "${Env:MinGW32}"
& "${PSScriptRoot}\build_windows_mingw_any.ps1" -BuildArch "x86_64" -Toolchain "${Env:MinGW64}"
