# 注意文件格式，编码必须为UTF8-BOM
[CmdletBinding()]
# ===== 配置参数信息 =====
param (
    # 编译架构（i686、x86_64）
    [string] $BuildArch = "i686",
    # 工具链
    [string] $Toolchain = (Resolve-Path -Path "${Env:MinGW32}")
)

begin {
    # ===== 变量预声明 =====
    # 定义输出编码（对[Console]::WriteLine生效）
    $OutputEncoding = [console]::InputEncoding = [console]::OutputEncoding = [Text.UTF8Encoding]::UTF8
    # 遇到错误立即停止
    $ErrorActionPreference = 'Stop'
    # 配置项目目录
    $projectDir = (Resolve-Path "${PSScriptRoot}\..\").Path
    # 配置构建目录
    $buildDir = "${projectDir}\build"
    # 配置发布目录
    $publishDir = "${projectDir}\dist\mingw"
    # 编译类型（Debug、Release、RelWithDebInfo、MinSizeRel）
    # Debug: 由于没有优化和完整的调试信息，这通常在开发和调试期间使用，因为它通常提供最快的构建时间和最佳的交互式调试体验。
    # Release: 这种构建类型通常快速的提供了充分的优化，并且没有调试信息，尽管一些平台在某些情况下仍然可能生成调试符号。
    # RelWithDebInfo: 这在某种程度上是前两者的折衷。它的目标是使性能接近于发布版本，但仍然允许一定程度的调试。
    # MinSizeRel: 这种构建类型通常只用于受限制的资源环境，如嵌入式设备。
    $buildType = "Release"
    # 移除旧的构建目录
    Remove-Item -Path "${buildDir}" -Recurse -Force -ErrorAction Ignore
    # 创建新的构建目录
    New-Item -Path "${buildDir}" -ItemType Directory
}

process {
    # 构建开始
    Write-Host "------------------------------- 构建:开始 -------------------------------"
    Write-Host "[编译器:${Toolchain}\bin\${BuildArch}-w64-mingw32-gcc]"
    Write-Host "[架构:${BuildArch}]"
    Write-Host "[模式:${buildType}]"
    
    # 执行CMake
    Write-Host "------------------------------- 执行CMake -------------------------------"
    cmake -G "MinGW Makefiles" `
        -DTOOLCHAIN_PATH="${Toolchain}" `
        -DCMAKE_SYSTEM_PROCESSOR="${BuildArch}" `
        -DCMAKE_TOOLCHAIN_FILE="${projectDir}/cmake-toolchains/windows-mingw-toolchain.cmake" `
        -DCMAKE_BUILD_TYPE="${buildType}" `
        -S "${projectDir}" `
        -B "${buildDir}"

    # 执行make
    Write-Host "------------------------------- 执行Make -------------------------------"
    cmake --build "${buildDir}" --config "${buildType}"

    # 执行make install
    Write-Host "--------------------------- 执行Make Install ---------------------------"
    cmake --install "${buildDir}" --config "${buildType}" --prefix "${publishDir}"

    # 执行压缩
    Compress-Archive -Force -Path "${publishDir}\libbecamdshow_windows_${BuildArch}\*" -DestinationPath "${projectDir}\dist\libbecamdshow_windows_${BuildArch}_mingw.zip"
    # 执行压缩
    Compress-Archive -Force -Path "${publishDir}\libbecammf_windows_${BuildArch}\*" -DestinationPath "${projectDir}\dist\libbecammf_windows_${BuildArch}_mingw.zip"
}

end {
    # 构建结束
    Write-Host "------------------------------- 构建:结束 -------------------------------"
}
