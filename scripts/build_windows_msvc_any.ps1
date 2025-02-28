# 注意文件格式，编码必须为UTF8-BOM
[CmdletBinding()]
# ===== 配置参数信息 =====
param (
    # 编译架构（i686 | x86_64 | arm | aarch64
    [Parameter(Mandatory = $false)]
    [ValidateSet("i686", "x86_64", "arm", "aarch64")]
    [string] $BuildArch = "i686",
    # 编译版本号
    [Parameter(Mandatory = $true)]
    [string] $BuildVersion = "2.0.0.0",
    # VS生成工具版本（Visual Studio 16 2019 | Visual Studio 17 2022）
    [Parameter(Mandatory = $false)]
    [ValidateSet("Visual Studio 16 2019", "Visual Studio 17 2022")]
    [string] $VsVersion = "Visual Studio 16 2019"
)

# 保存旧的PATH
$oldPath = $Env:Path
try {
    # ===== 变量预声明 =====
    # 定义输出编码（对[Console]::WriteLine生效）
    $OutputEncoding = [console]::InputEncoding = [console]::OutputEncoding = [Text.UTF8Encoding]::UTF8
    # 遇到错误立即停止
    $ErrorActionPreference = 'Stop'
    # 配置项目目录
    $projectDir = (Resolve-Path "${PSScriptRoot}\..\").Path
    # 配置构建目录
    $buildDir = "${projectDir}\build"
    # 配置安装目录
    $installDir = "${projectDir}\install\msvc"
    # 配置发布目录
    $publishDir = "${projectDir}\publish"
    # 编译类型（Debug、Release、RelWithDebInfo、MinSizeRel）
    # Debug: 由于没有优化和完整的调试信息，这通常在开发和调试期间使用，因为它通常提供最快的构建时间和最佳的交互式调试体验。
    # Release: 这种构建类型通常快速的提供了充分的优化，并且没有调试信息，尽管一些平台在某些情况下仍然可能生成调试符号。
    # RelWithDebInfo: 这在某种程度上是前两者的折衷。它的目标是使性能接近于发布版本，但仍然允许一定程度的调试。
    # MinSizeRel: 这种构建类型通常只用于受限制的资源环境，如嵌入式设备。
    $buildType = "Release"
    # 移除旧的构建目录
    Remove-Item -Path "${buildDir}" -Recurse -Force -ErrorAction Ignore
    # 创建新的构建目录
    New-Item -Path "${buildDir}" -ItemType Directory -Force
    if (!(Test-Path -Path "${installDir}")) {
        New-Item -Path "${installDir}" -ItemType Directory
    }
    if (!(Test-Path -Path "${publishDir}")) {
        New-Item -Path "${publishDir}" -ItemType Directory
    }

    # 设置 vswhere 的路径
    $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    # 检查 vswhere 是否存在
    if (-Not (Test-Path $vswherePath)) {
        throw "vswhere.exe not found. Please install Visual Studio or download vswhere from GitHub."
    }

    # 根据输入的字符串确定版本范围
    if ($VsVersion -eq "Visual Studio 16 2019") {
        $versionRange = "[16.0,17.0)"
    }
    elseif ($VsVersion -eq "Visual Studio 17 2022") {
        $versionRange = "[17.0,18.0)"
    }
    else {
        throw "Unsupported Visual Studio version: ${VsVersion}"
    }

    # 查询 Visual Studio 的安装路径
    $vsPath = & $vswherePath -version $versionRange -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    # 检查是否找到 Visual Studio 的安装路径
    if (-Not $vsPath) {
        throw "${VsVersion} installation not found."
    }
    # 输出 Visual Studio 的安装路径
    Write-Host "${VsVersion} installation path: $vsPath"

    # 查询 MSVC 工具链路径
    $msvcToolchainPath = & $vswherePath -version $versionRange -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "VC\Tools\MSVC\**\cl.exe"
    # 检查是否找到 MSVC 工具链路径
    if (-Not $msvcToolchainPath) {
        throw "MSVC toolchain not found in $vsVersionString."
    }
    # 输出 MSVC 工具链路径
    Write-Host "MSVC toolchain path: $msvcToolchainPath"

    # 架构转换
    switch ($BuildArch) {
        "i686" { 
            $vcHost = "x86"
            $vcArch = "win32"
        }
        "x86_64" { 
            $vcHost = "x64"
            $vcArch = "x64"
        }
        "arm" { 
            $vcHost = "x64"
            $vcArch = "arm"
        }
        "aarch64" { 
            $vcHost = "x64"
            $vcArch = "arm64"
        }
        Default {
            Throw "不支持的额目标架构：${$BuildArch}"
        }
    }
    # VS提供的环境配置脚本配置编译环境
    & "${vsPath}\VC\Auxiliary\Build\vcvarsall.bat" $vcHost


    # 构建开始
    Write-Host "--------------------------------- 构建:开始 ----------------------------------"
    Write-Host "[编译器:${msvcToolchainPath}]"
    Write-Host "[架构:${BuildArch}]"
    Write-Host "[模式:${buildType}]"
    
    # # 执行CMake
    Write-Host "------------------------------- 执行CMake:配置 -------------------------------"
    cmake -G "${VsVersion}" `
        -DCMAKE_SYSTEM_NAME="Windows" `
        -DCMAKE_SYSTEM_PROCESSOR="${BuildArch}" `
        -S"${projectDir}" `
        -B"${buildDir}" `
        -T host="${vcHost}" `
        -A "${vcArch}"

    # 执行make
    Write-Host "------------------------------- 执行CMake:构建 -------------------------------"
    cmake --build "${buildDir}" --config "${buildType}"

    # 执行make install
    Write-Host "------------------------------- 执行CMake:安装 -------------------------------"
    cmake --install "${buildDir}" --config "${buildType}" --prefix "${installDir}"

    Write-Host "---------------------------------- 执行压缩 ----------------------------------"
    # 不支持Direct Show，所以仅压缩Miedia Foundation
    # 拷贝pkg-config配置文件，并赋值版本号
    $mfPcContent = (Get-Content -Path "${installDir}\libbecam_windows_${BuildArch}_mf\becam.pc") -creplace "ENV_LIBRARY_VERSION", "${BuildVersion}"
    $mfPcContent | Set-Content -Path "${installDir}\libbecam_windows_${BuildArch}_mf\becam.pc" -Force
    # 执行压缩
    Compress-Archive -Force -Path "${installDir}\libbecam_windows_${BuildArch}_mf\*" -DestinationPath "${publishDir}\libbecam_windows_${BuildArch}_mf_msvc.zip"

    # 构建结束
    Write-Host "--------------------------------- 构建:结束 ----------------------------------"
}
finally {
    # 还原旧的PATH
    $Env:Path = $oldPath
}
