# 配置相关源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 声明最低支持的平台为Win7（Win8才支持低延迟配置）
add_compile_definitions(_WIN32_WINNT=0x0601)

# 配置链接库
link_libraries(ole32 mf mfplat mfuuid mfreadwrite)

# 配置静态库生成参数
add_library(becammf_static STATIC ${SOURCES})
set_target_properties(becammf_static PROPERTIES OUTPUT_NAME "becammf")
# 配置动态库生成参数
add_library(becammf_shared SHARED ${SOURCES})
set_target_properties(becammf_shared PROPERTIES OUTPUT_NAME "becammf")

# 指定make install后头文件、静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecam_${BUILD_OS}_${BUILD_ARCH}_mf)
# 头文件
install(FILES ${CMAKE_SOURCE_DIR}/include/becam/becam.h DESTINATION ${INSTALL_PATH}/include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/becam_${TOOLCHAIN_VENDER}.pc DESTINATION ${INSTALL_PATH} RENAME "becam.pc")
# ARCHIVE-静态库，LIBRARY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becammf_static ARCHIVE DESTINATION ${INSTALL_PATH}/lib/static)
install(TARGETS becammf_shared LIBRARY DESTINATION ${INSTALL_PATH}/lib/shared
                               RUNTIME DESTINATION ${INSTALL_PATH}/lib/shared)
