# 配置相关源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 配置静态库生成参数
add_library(becamuvc_static STATIC ${SOURCES})
target_link_libraries(becamuvc_static PRIVATE uvc_static)
set_target_properties(becamuvc_static PROPERTIES OUTPUT_NAME "becamuvc")
# 配置动态库生成参数
add_library(becamuvc_shared SHARED ${SOURCES})
target_link_libraries(becamuvc_shared PRIVATE uvc_static)
set_target_properties(becamuvc_shared PROPERTIES OUTPUT_NAME "becamuvc")

# 指定make install后头文件、静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecamuvc_${BUILD_OS}_${BUILD_ARCH})
# 头文件
install(FILES ${CMAKE_SOURCE_DIR}/include/becam/becam.h DESTINATION ${INSTALL_PATH}/include)
# ARCHIVE-静态库，LIBRARY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becamuvc_static ARCHIVE DESTINATION ${INSTALL_PATH}/lib)
install(TARGETS becamuvc_shared LIBRARY DESTINATION ${INSTALL_PATH}/lib)
