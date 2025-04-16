# 设置目标系统
set(CMAKE_SYSTEM_NAME Linux)

# 打印信息
message(STATUS "TOOLCHAIN_PATH: ${TOOLCHAIN_PATH}")
message(STATUS "TOOLCHAIN_COMPILER_NAME: ${TOOLCHAIN_COMPILER_NAME}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

# 指定交叉编译器
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_COMPILER_NAME}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_COMPILER_NAME}-g++)

# 指定工具链根目录
# 通常可以这样查询：${TOOLCHAIN_PATH}/bin/${CMAKE_SYSTEM_PROCESSOR}-linux-gnueabihf-gcc -print-sysroot
# set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH}/${CMAKE_SYSTEM_PROCESSOR}-linux-gnueabihf/libc)

# 只在工具链目录中查找库和头文件
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
