# Becam —— 跨平台相机库
`Becam`致力于提供跨平台的极简`Camera`调用，将复杂的系统调用包装为统一的接口，屏蔽系统之间的调用差异，使你的项目注重业务逻辑，不必浪费更多的时间在这里。

## 平台支持
| 平台 | 媒体库 | 状态 |
| :---------: | :--------: | :--------: |
| Windows | Direct Show | `Supported` |
| Windows | Media Foundation | `Supported` |
| Linux | V4L2 | `Supported` |
| UVC | libuvc | `Developing` |
| Android | Camera V1 | `Schedule` |
| Android | Camera V2 | `Schedule` |
| Harmony | Camera | `Schedule` |

## 提供的成品库所使用的工具链
| 平台 | 工具 | 版本 | 备注 |
| :---------: | :--------: | :--------: | :-------- |
| Windows | MinGW | `w64devkit-1.18.0` | 需要安装[MinGW](https://www.mingw-w64.org/downloads/) |
| Windows | MSVC | `Visual Studio 17 2022` | 需安装[Windows SDK](https://developer.microsoft.com/zh-cn/windows/downloads/windows-sdk/)，不支持编译`Direct Show`实现，因为可维护的Direct Show SDK很少，建议使用`MinGW`编译`Direct Show` |
| Linux | V4L2 | `GCC 8.5+` | 建议使用`8.3及以上版本的GCC` |
| UVC | libuvc | `-` | |
| Android | Camera V1 | `-` | |
| Android | Camera V2 | `-` | |
| Harmony | Camera | `-` | |