cmake_minimum_required(VERSION 3.5)

include(cmake/config.cmake)

project(libraptorq VERSION ${RAPTORQ_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# 生成 version 文件
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/version.h.in
    ${PROJECT_SOURCE_DIR}/cmake/version.h
    NEWLINE_STYLE UNIX
)

option(RAPTORQ_DYNAMIC_LIB "Build dynamic library" ON)
option(RAPTORQ_STATIC_LIB "Build static library" OFF)
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

# message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
# message(STATUS "CMAKE_INSTALL_FULL_BINDIR: ${CMAKE_INSTALL_FULL_BINDIR}")
# message(STATUS "CMAKE_INSTALL_FULL_LIBDIR : ${CMAKE_INSTALL_FULL_LIBDIR}")
# message(STATUS "CMAKE_INSTALL_FULL_INCLUDEDIR: ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
# message(STATUS "CMAKE_INSTALL_FULL_DATADIR: ${CMAKE_INSTALL_FULL_DATADIR}")
# message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
# message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

SET(SOURCES	src/decoder.cpp
        src/encoder.cpp
        src/raptorq.cpp)

SET(HEADERS	include/raptorq/decoder.h
        include/raptorq/encoder.h
        include/raptorq/raptorq.h)

set(CXX_FLAGS "-W -Wall -Werror -Wextra")
set(RAPTORQ_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBRAPTORQ_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libRaptorQ_v0_1_9/src)

# 添加依赖的库
add_subdirectory(libRaptorQ_v0_1_9)

# build the static library
if(RAPTORQ_STATIC_LIB MATCHES "ON")
    add_library(raptorq_static STATIC ${SOURCES} ${HEADERS})
    add_library(raptorq::raptorq_static ALIAS raptorq_static)

    target_include_directories(raptorq_static PRIVATE ${RAPTORQ_INC_DIR})
    target_include_directories(raptorq_static PRIVATE ${LIBRAPTORQ_INC_DIR})

    target_link_libraries(raptorq_static PRIVATE RaptorQ::RaptorQ_Static m pthread dl)

    target_compile_options(
        raptorq_static PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    set_target_properties (
        raptorq_static
        PROPERTIES
        LINK_FLAGS "-Wl,-z,now,-z,relro -pie"
    )
    set_property(TARGET raptorq_static PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    set(RAPTOR_LIBRARIE raptorq_static CACHE INTERNAL "link raptorq librarie")

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS raptorq_static
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

#build dynamic library
if(RAPTORQ_DYNAMIC_LIB MATCHES "ON")
    add_definitions(-DRAPTORQ_DLL)
    add_definitions(-DRAPTORQ_DLL_EXPORTS)

    add_library(raptorq SHARED ${SOURCES} ${HEADERS})
    add_library(raptorq::raptorq ALIAS raptorq)

    target_include_directories(raptorq PRIVATE ${RAPTORQ_INC_DIR})
    target_include_directories(raptorq PRIVATE ${LIBRAPTORQ_INC_DIR})

    target_link_libraries(raptorq PRIVATE RaptorQ::RaptorQ_Static m pthread dl)

    target_compile_options(
        raptorq PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    set_target_properties (
        raptorq
        PROPERTIES
        LINK_FLAGS "-Wl,-z,now,-z,relro"
        VERSION ${RAPTORQ_VERSION}
    )

    set_property(TARGET raptorq PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    set(RAPTOR_LIBRARIE raptorq CACHE INTERNAL "link raptorq librarie")
    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS raptorq
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${HEADERS} DESTINATION "${INSTALL_INC_DIR}/raptorq")
endif()