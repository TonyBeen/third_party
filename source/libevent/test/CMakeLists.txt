list(APPEND EXAMPLES
    test_event_loop
)

add_executable(test_event_loop test_event_loop.cc)
if(EVENT_STATIC_LIB MATCHES "ON")
target_link_libraries(test_event_loop event_wrapper_static)
elseif(EVENT_DYNAMIC_LIB MATCHES "ON")
target_link_libraries(test_event_loop event_wrapper)
endif()

# add_executable(complex_example complex_example.cc)
# if(EVENT_STATIC_LIB MATCHES "ON")
# target_link_libraries(complex_example event_wrapper_static)
# elseif(EVENT_DYNAMIC_LIB MATCHES "ON")
# target_link_libraries(complex_example event_wrapper)
# endif()

add_custom_target(examples DEPENDS ${EXAMPLES})