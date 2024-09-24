list(APPEND EXAMPLES
    example
    complex_example
)

include_directories(${RAPTORQ_INC_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(example example.cc)
target_link_libraries(example msgpack)

add_executable(complex_example complex_example.cc)
target_link_libraries(complex_example msgpack)

add_custom_target(examples DEPENDS ${EXAMPLES})
