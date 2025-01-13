& "${PSScriptRoot}\build_windows_mingw_any.ps1" `
    -BuildType Release `
    -BuildArch i686 `
    -Toolchain "${Env:MinGW32}"
