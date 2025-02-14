cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(libevent_wrapper VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

include(GNUInstallDirs)

# default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo") # Release MinSizeRel RelWithDebInfo Debug
endif()

option(EVENT_DYNAMIC_LIB "Build dynamic library" ON)
option(EVENT_STATIC_LIB "Build static library" ON)
option(BUILD_EXAMPLES "Compile sample program" OFF)
option(BUILD_TEST "Compile sample program" ON)
option(ENABLE_DEBUG "Enable debug symbol" OFF)

if(ENABLE_DEBUG MATCHES "ON")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

set(SOURCES
    src/dns_resolve.cpp
    src/event_async.cpp
    src/event_base.cpp
    src/event_loop.cpp
    src/event_poll.cpp
    src/event_signal.cpp
    src/event_timer.cpp)

set(HEADERS
    include/event/dns_resolve.h
    include/event/event_async.h
    include/event/event_base.h
    include/event/event_loop.h
    include/event/event_poll.h
    include/event/event_signal.h
    include/event/event_timer.h)

# libevent
add_subdirectory(libevent_V2_1_10)
# 异步DNS解析库
add_subdirectory(libares_V1_34_1)
# nghttp2
add_subdirectory(nghttp2_V1_61_0)

# build the static library
if(EVENT_STATIC_LIB MATCHES "ON")
    add_library(event_wrapper_static STATIC ${SOURCES} ${HEADERS})
    # add_library(event_wrapper::event_wrapper_static ALIAS event_wrapper_static)

    target_include_directories(event_wrapper_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(event_wrapper_static PUBLIC c-ares::cares_static event)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(event_wrapper_static PUBLIC pthread)
    endif()

    target_compile_options(
        event_wrapper_static PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set_target_properties (
            event_wrapper_static
            PROPERTIES
            LINK_FLAGS "-Wl,-z,now,-z,relro -pie"
        )
    endif()

    set_property(TARGET event_wrapper_static PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS event_wrapper_static
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

#build dynamic library
if(EVENT_DYNAMIC_LIB MATCHES "ON")
    add_library(event_wrapper SHARED ${SOURCES} ${HEADERS})
    # add_library(event_wrapper::event_wrapper ALIAS event_wrapper)

    target_include_directories(event_wrapper
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(event_wrapper PUBLIC c-ares::cares_static event)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(event_wrapper_static PUBLIC pthread)
    endif()

    target_compile_options(
        event_wrapper PRIVATE
        ${CXX_COMPILER_FLAGS}
    )

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set_target_properties (
            event_wrapper
            PROPERTIES
            LINK_FLAGS "-Wl,-z,now,-z,relro -pie"
        )
    endif ()

    set_property(TARGET event_wrapper PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS event_wrapper
            RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
            ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
            LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
    endif()
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TEST)
    add_subdirectory(test)
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${HEADERS} DESTINATION "${INSTALL_INC_DIR}/event")
endif()

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)
endif()
