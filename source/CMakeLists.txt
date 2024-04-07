cmake_minimum_required(VERSION 3.0)

project(third_party)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")

add_subdirectory(libhv)
add_subdirectory(hiredis)
add_subdirectory(yaml-cpp)
add_subdirectory(zlib)

# protobuf依赖zlib
# add_subdirectory(protobuf)
# add_subdirectory(sqlite)