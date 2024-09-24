cmake_minimum_required(VERSION 3.5)

project(libmsgpack VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

option(MSGPACK_DYNAMIC_LIB "Build dynamic library" ON)
option(MSGPACK_STATIC_LIB "Build static library" ON)
option(BUILD_EXAMPLES "Compile sample program" ON)
option(ENABLE_DEBUG "Enable debug symbol" ON)

# default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo") # Release MinSizeRel RelWithDebInfo Debug
endif()

if(ENABLE_DEBUG MATCHES "ON")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

set(SOURCES	src/msgpackpp.cpp)
set(HEADERS	include/msgpackpp.hpp)

set(CXX_FLAGS "-W -Wall -Werror -Wextra")

# 添加依赖的库
add_subdirectory(msgpack_c)

# build the static library
if(MSGPACK_STATIC_LIB MATCHES "ON")
    add_library(msgpack_static STATIC ${SOURCES} ${HEADERS})
    # add_library(msgpack::msgpack_static ALIAS msgpack_static)

    # target_include_directories(msgpack_static PRIVATE ${RAPTORQ_INC_DIR})
    # target_include_directories(msgpack_static PRIVATE ${LIBRAPTORQ_INC_DIR})

    target_include_directories(msgpack_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(msgpack_static PUBLIC msgpack-c pthread)

    target_compile_options(
        msgpack_static PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    set_target_properties (
        msgpack_static
        PROPERTIES
        LINK_FLAGS "-Wl,-z,now,-z,relro -pie"
    )
    set_property(TARGET msgpack_static PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    set(RAPTOR_LIBRARIE msgpack_static CACHE INTERNAL "link msgpack librarie")

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS msgpack_static
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

#build dynamic library
if(MSGPACK_DYNAMIC_LIB MATCHES "ON")
    add_library(msgpack SHARED ${SOURCES} ${HEADERS})
    # add_library(msgpack::msgpack ALIAS msgpack)

    target_include_directories(msgpack
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(msgpack PUBLIC msgpack-c pthread)

    target_compile_options(
        msgpack PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    set_target_properties (
        msgpack
        PROPERTIES
        LINK_FLAGS "-Wl,-z,now,-z,relro"
    )

    set_property(TARGET msgpack PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    set(RAPTOR_LIBRARIE msgpack CACHE INTERNAL "link msgpack librarie")
    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS msgpack
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${HEADERS} DESTINATION "${INSTALL_INC_DIR}")
endif()