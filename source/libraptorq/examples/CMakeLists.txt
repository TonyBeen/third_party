list(APPEND EXAMPLES
    example_encode_file
    example_udp_server
    example_udp_client
)

include_directories(${RAPTORQ_INC_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example_encode_file example_encode_file.c)
target_link_libraries(example_encode_file ${RAPTOR_LIBRARIE})

add_executable(example_udp_server udp_server.cc socket_utils.cc crc32.cpp)
target_link_libraries(example_udp_server ${RAPTOR_LIBRARIE})

add_executable(example_udp_client udp_client.cc socket_utils.cc crc32.cpp)
target_link_libraries(example_udp_client ${RAPTOR_LIBRARIE})

add_custom_target(examples DEPENDS ${EXAMPLES})
