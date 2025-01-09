# 注意文件格式，编码必须为UTF8-BOM
[CmdletBinding()]
# ===== 配置参数信息 =====
param (
    # 编译类型（Debug、Release、RelWithDebInfo、MinSizeRel）
    # Debug: 由于没有优化和完整的调试信息，这通常在开发和调试期间使用，因为它通常提供最快的构建时间和最佳的交互式调试体验。
    # Release: 这种构建类型通常快速的提供了充分的优化，并且没有调试信息，尽管一些平台在某些情况下仍然可能生成调试符号。
    # RelWithDebInfo: 这在某种程度上是前两者的折衷。它的目标是使性能接近于发布版本，但仍然允许一定程度的调试。
    # MinSizeRel: 这种构建类型通常只用于受限制的资源环境，如嵌入式设备。
    [string] $BuildType = "Debug",
    # 编译架构（i686、x86_64）
    [string] $BuildArch = "i686",
    # 工具链
    [string] $Toolchain = (Resolve-Path -Path "C:\MinGW\mingw32")
)

begin {
    # ===== 变量预声明 =====
    # 定义输出编码（对[Console]::WriteLine生效）
    $OutputEncoding = [console]::InputEncoding = [console]::OutputEncoding = [Text.UTF8Encoding]::UTF8
    # 遇到错误立即停止
    $ErrorActionPreference = 'Stop'
    # 配置项目根目录
    $ProjectRootPath = (Resolve-Path "${PSScriptRoot}").Path

    # 指定编译器
    $Compiler = "${BuildArch}-w64-mingw32"
    # 将工具链添加到环境变量
    $ToolchainBin = Resolve-Path -Path "${Toolchain}\bin"
    if (!$Env:Path.StartsWith($ToolchainBin)) {
        $Env:Path = "${ToolchainBin};${Env:Path}"
    }
    Write-Host "环境变量：${Env:Path}"

    Write-Host "----------------------------- CMake版本信息 -----------------------------"
    cmake --version
    Write-Host "------------------------------ GCC版本信息 ------------------------------"
    gcc --version
    Write-Host "------------------------------ G++版本信息 ------------------------------"
    g++ --version
}

process {
    # 移除旧的构建目录
    Remove-Item -Path "${ProjectRootPath}/build" -Recurse -Force -ErrorAction Ignore
    # 创建新的构建目录
    New-Item -Path "${ProjectRootPath}/build" -ItemType Directory

    # 构建开始
    Write-Host "------------------------------- 构建:开始 -------------------------------"
    Write-Host "[编译器:${Compiler}]"
    Write-Host "[架构:${BuildArch}]"
    Write-Host "[模式:${BuildType}]"
    
    # 执行CMake
    Write-Host "------------------------------- 执行CMake -------------------------------"
    cmake -G "MinGW Makefiles" `
        -DCMAKE_SYSTEM_NAME="Windows" `
        -DCMAKE_SYSTEM_PROCESSOR="${BuildArch}" `
        -DCMAKE_BUILD_TYPE="${BuildType}" `
        -DCMAKE_C_COMPILER="${Compiler}-gcc" `
        -DCMAKE_CXX_COMPILER="${Compiler}-g++" `
        -S "${ProjectRootPath}" `
        -B "${ProjectRootPath}/build"

    # 执行make
    Write-Host "------------------------------- 执行Make -------------------------------"
    cmake --build "${ProjectRootPath}/build" --config "${BuildType}"

    # 执行make install
    Write-Host "--------------------------- 执行Make Install ---------------------------"
    cmake --install "${ProjectRootPath}/build" --config "${BuildType}"

    # 执行压缩
    Compress-Archive -Path "${ProjectRootPath}/dist/libbecamdshow_windows_${BuildArch}/*" -DestinationPath "${ProjectRootPath}/dist/libbecamdshow_windows_${BuildArch}.zip"
}

end {
    # 构建结束
    Write-Host "------------------------------- 构建:结束 -------------------------------"
}
