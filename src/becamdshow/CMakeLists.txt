# 配置相关源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 配置链接库
link_libraries(strmiids ole32 oleaut32)

# 配置静态库生成参数
add_library(becamdshow_static STATIC ${SOURCES})
set_target_properties(becamdshow_static PROPERTIES OUTPUT_NAME "becamdshow-static")
# 配置动态库生成参数
add_library(becamdshow_shared SHARED ${SOURCES})
set_target_properties(becamdshow_shared PROPERTIES OUTPUT_NAME "becamdshow")
target_compile_definitions(becamdshow_shared PRIVATE BECAM_SHARED BECAM_SHARED_EXPORT)

# 指定make install后头文件、静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecam_${BUILD_OS}_${BUILD_ARCH}_dshow)
# 头文件
install(FILES ${CMAKE_SOURCE_DIR}/include/becam/becam.h DESTINATION ${INSTALL_PATH}/include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/becam_${TOOLCHAIN_VENDER}.pc DESTINATION ${INSTALL_PATH} RENAME "becam.pc")
# ARCHIVE-静态库，LIBRARY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becamdshow_static ARCHIVE DESTINATION ${INSTALL_PATH}/lib)
install(TARGETS becamdshow_shared LIBRARY DESTINATION ${INSTALL_PATH}/lib
                                  RUNTIME DESTINATION ${INSTALL_PATH}/lib)
