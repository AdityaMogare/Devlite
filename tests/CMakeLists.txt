include(GoogleTest)

add_executable(devlite_tests
  unit/fake_environment.hpp
  unit/path_resolver_test.cpp
  unit/process_runner_test.cpp
  unit/rules_test.cpp
  unit/format_test.cpp)
target_link_libraries(devlite_tests PRIVATE devlite_core GTest::gtest_main)
target_include_directories(devlite_tests PRIVATE unit)
target_compile_definitions(devlite_tests PRIVATE
  DEVLITE_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden")
set_source_files_properties(unit/fake_environment.hpp PROPERTIES HEADER_FILE_ONLY ON)

gtest_discover_tests(devlite_tests)
