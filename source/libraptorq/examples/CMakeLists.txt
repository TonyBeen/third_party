list(APPEND EXAMPLES
    example_encode_file
)

include_directories(${RAPTORQ_INC_DIR})

add_executable(example_encode_file example_encode_file.c)
target_link_libraries(example_encode_file ${RAPTOR_LIBRARIE})

add_custom_target(examples DEPENDS ${EXAMPLES})
