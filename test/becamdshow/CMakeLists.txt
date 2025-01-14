# 编译为可执行文件
add_executable(becamdshow_init_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_init_test.cpp)
add_executable(becamdshow_get_list_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_get_list_test.cpp)
add_executable(becamdshow_open_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_open_test.cpp)
add_executable(becamdshow_frame_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_frame_test.cpp)
add_executable(becamdshow_all_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_all_test.cpp)

# 指定需要链接的库
# 链接到静态库（注意：库名需要填写cmake add_library函数的第一个参数name，否则找不到库）
target_link_libraries(becamdshow_init_test becamdshow_static)
target_link_libraries(becamdshow_get_list_test becamdshow_static)
target_link_libraries(becamdshow_open_test becamdshow_static)
target_link_libraries(becamdshow_frame_test becamdshow_static)
target_link_libraries(becamdshow_all_test becamdshow_static)

# 指定make install后静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecamdshow_${BUILD_OS}_${BUILD_ARCH})
# ARCHIVE-静态库，LIBRATY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becamdshow_init_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becamdshow_get_list_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becamdshow_open_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becamdshow_frame_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becamdshow_all_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)