# 配置相关源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 配置静态库生成参数
add_library(becamv4l2_static STATIC ${SOURCES})
set_target_properties(becamv4l2_static PROPERTIES OUTPUT_NAME "becamv4l2")
# 配置动态库生成参数
add_library(becamv4l2_shared SHARED ${SOURCES})
set_target_properties(becamv4l2_shared PROPERTIES OUTPUT_NAME "becamv4l2")

# 指定make install后头文件、静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecam_${BUILD_OS}_${BUILD_ARCH}_v4l2)
# 头文件
install(FILES ${CMAKE_SOURCE_DIR}/include/becam/becam.h DESTINATION ${INSTALL_PATH}/include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/becam.pc DESTINATION ${INSTALL_PATH})
# ARCHIVE-静态库，LIBRARY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becamv4l2_static ARCHIVE DESTINATION ${INSTALL_PATH}/lib)
install(TARGETS becamv4l2_shared LIBRARY DESTINATION ${INSTALL_PATH}/lib)
