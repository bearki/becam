& "${PSScriptRoot}\build_windows_any.ps1" `
    -BuildType Release `
    -BuildArch x86_64 `
    -Toolchain "${Env:MinGW64}"
