cmake_minimum_required(VERSION 3.14)
project(pico-llm-viewer-tests CXX)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_library(viewer_testable STATIC
    ../src/display_usage/display_usage_use_case.cpp
    ../src/adapters/claude/claude_usage_json_parser.cpp
)
target_include_directories(viewer_testable PUBLIC ../src)

add_executable(viewer_tests
    application/display_usage_use_case_test.cpp
    adapters/usage_json_parser_test.cpp
)
target_link_libraries(viewer_tests
    viewer_testable
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(viewer_tests)
