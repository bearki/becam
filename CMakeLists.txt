# 指定Cmake工具所需的最小版本
cmake_minimum_required(VERSION 3.10)

# 配置项目名
project(Becam)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

#告知当前使用的是交叉编译方式，必须配置
if (NOT CMAKE_SYSTEM_NAME)
    # 设置目标操作系统名称
    if(WIN32) 
        set(CMAKE_SYSTEM_NAME Windows)
    elseif(LINUX)
        set(CMAKE_SYSTEM_NAME Linux)
    endif()
    # 设置目标操作系统架构
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif()

# 修复MSVC编译警告
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 设置编译后的静态库文件的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
# 设置编译后的动态库文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
# 设置编译后的可执行文件的输出成目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
# 命令行未设置安装前缀时内部主动设置
# 设定编译后生成的制品文件安装目录前缀
if(MSVC) 
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/msvc)
elseif(MINGW)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/mingw)
else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/gnu)
endif()

# 配置引用头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/src)
# 配置引用库链接目录（静态库或动态库都行）
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# 转换一下大小写方便打包
string(TOLOWER "${CMAKE_SYSTEM_NAME}" BUILD_OS)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" BUILD_ARCH)

# 打印一下系统类型
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${BUILD_ARCH}")


############################ 通用平台的相机库实现 ############################
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # 启用CMake新规则，否则变量传不进去
    if(POLICY CMP0079)
        cmake_policy(SET CMP0079 NEW)
    endif()

    # 引入三方库libusb源码
    # set(LIBUSB_BUILD_SHARED_LIBS TRUE) # 添加这一行可以把libusb编译为动态库
    set(LIBUSB_BUILD_SHARED_LIBS OFF) # 将libusb编译为静态库
    set(LIBUSB_BUILD_EXAMPLES ON) # 编译示例程序
    set(LIBUSB_INSTALL_TARGETS OFF) # 不需要安装目标
    set(LibUSB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include/libusb-cmake/libusb/libusb) # 指定libusb头文件目录
    set(LibUSB_LIBRARY_DIRS ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}) # 指定libusb编译后的库目录
    add_subdirectory(${CMAKE_SOURCE_DIR}/include/libusb-cmake) # 添加libusb子目录
    set_target_properties(usb-1.0 PROPERTIES POSITION_INDEPENDENT_CODE ON) # 将静态库设置为位置无关，既：-fPIC

    # 导入编译好的libusb供libuvc使用
    add_library(LibUSB::LibUSB STATIC IMPORTED) # 导入LibUSB::LibUSB库
    set_target_properties(LibUSB::LibUSB PROPERTIES 
                        INTERFACE_INCLUDE_DIRECTORIES ${LibUSB_INCLUDE_DIRS}) # 指定LibUSB::LibUSB头文件目录
    set_target_properties(LibUSB::LibUSB PROPERTIES
                        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                        IMPORTED_LOCATION "${LibUSB_LIBRARY_DIRS}/libusb-1.0.a") # 指定LibUSB::LibUSB库文件
                        
    # 引入三方库libuvc源码
    set(CMAKE_BUILD_TARGET "Shared") # 编译成静态库
    set(BUILD_EXAMPLE ON) # 编译实例程序
    add_subdirectory(${CMAKE_SOURCE_DIR}/include/libuvc) # 添加libuvc子目录

    # 强制等待libusb编译完成
    if(CMAKE_BUILD_TARGET MATCHES "Shared")
        target_link_libraries(uvc PRIVATE udev)
        add_dependencies(uvc usb-1.0)
        set(LibUVC_STATIC OFF) # libuvc的CMake写的有问题，必须启用，否则示例程序编译不通过
    elseif(CMAKE_BUILD_TARGET MATCHES "Static")
        add_dependencies(uvc_static usb-1.0)
        set(LibUVC_STATIC ON) # libuvc的CMake写的有问题，必须启用，否则示例程序编译不通过
    elseif(CMAKE_BUILD_TARGET MATCHES "Both")
        target_link_libraries(uvc PRIVATE udev)
        add_dependencies(uvc usb-1.0)
        add_dependencies(uvc_static usb-1.0)
        set(LibUVC_STATIC ON) # libuvc的CMake写的有问题，必须启用，否则示例程序编译不通过
    endif()
endif()

############################ 特定平台的相机库实现 ############################
# 根据条件添加子目录
if(CMAKE_SYSTEM_NAME MATCHES "Windows") # windows环境下需要编译的内容
    # 增加子目录
    if(MINGW) # 仅在MINGW环境下编译Direct Show
        add_subdirectory(${CMAKE_SOURCE_DIR}/src/becamdshow)
        add_subdirectory(${CMAKE_SOURCE_DIR}/test/becamdshow)
    endif()
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/becammf)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test/becammf)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux") # linux环境下需要编译的内容
    # 增加子目录
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/becamv4l2)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test/becamv4l2)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin") # mac环境下需要编译的内容
    # 增加子目录
else()
    message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}, 暂无匹配该OS的动作")
endif()
