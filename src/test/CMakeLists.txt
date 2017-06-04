add_executable(interval_tests interval_tests.cpp)
target_link_libraries(interval_tests mtidd)
add_dependencies(interval_tests CATCH)
add_test(interval_tests ${CMAKE_CURRENT_BINARY_DIR}/interval_tests)

add_executable(lattice_tests lattice_tests.cpp)
target_link_libraries(lattice_tests mtidd)
add_dependencies(lattice_tests CATCH)
add_test(lattice_tests ${CMAKE_CURRENT_BINARY_DIR}/lattice_tests)

add_executable(internalizer_tests internalizer_tests.cpp)
target_link_libraries(internalizer_tests mtidd)
add_dependencies(internalizer_tests CATCH)
add_test(internalizer_tests ${CMAKE_CURRENT_BINARY_DIR}/internalizer_tests)

add_executable(partition_tests partition_tests.cpp)
target_link_libraries(partition_tests mtidd)
add_dependencies(partition_tests CATCH)
add_test(partition_tests ${CMAKE_CURRENT_BINARY_DIR}/partition_tests)

