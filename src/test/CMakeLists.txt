add_executable(interval_tests interval_tests.cpp)
target_link_libraries(interval_tests mtidd)
add_test(interval_tests ${CMAKE_CURRENT_BINARY_DIR}/interval_tests)

