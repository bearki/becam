# 编译为可执行文件
add_executable(becammf_init_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_init_test.cpp)
add_executable(becammf_get_list_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_get_list_test.cpp)
add_executable(becammf_open_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_open_test.cpp)
add_executable(becammf_frame_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_frame_test.cpp)
add_executable(becammf_all_test ${CMAKE_CURRENT_SOURCE_DIR}/../common/becam_all_test.cpp)

# 指定需要链接的库
# 链接到静态库（注意：库名需要填写cmake add_library函数的第一个参数name，否则找不到库）
target_link_libraries(becammf_init_test PRIVATE becammf_static)
target_link_libraries(becammf_get_list_test PRIVATE becammf_static)
target_link_libraries(becammf_open_test PRIVATE becammf_static)
target_link_libraries(becammf_frame_test PRIVATE becammf_static)
target_link_libraries(becammf_all_test PRIVATE becammf_static)

# 指定make install后静态库，动态库，可执行文件存放目录
set(INSTALL_PATH libbecam_${BUILD_OS}_${BUILD_ARCH}_mf)
# ARCHIVE-静态库，LIBRATY-动态库，RUNTIME-可执行二进制文件
install(TARGETS becammf_init_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becammf_get_list_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becammf_open_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becammf_frame_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)
install(TARGETS becammf_all_test RUNTIME DESTINATION ${INSTALL_PATH}/bin)