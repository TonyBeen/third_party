cmake_minimum_required(VERSION 3.0)

project(third_party)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")

add_subdirectory(libhv_V1_3_2)
add_subdirectory(hiredis_V1_1_0)
add_subdirectory(yaml_cpp_V0_8_0)
add_subdirectory(zlib_V1_3)
add_subdirectory(libRaptorQ_v0_1_9)
add_subdirectory(libcuckoo_master)

# protobuf依赖zlib
# add_subdirectory(protobuf_V3_21_12)