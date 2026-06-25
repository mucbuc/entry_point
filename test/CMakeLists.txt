cmake_minimum_required(VERSION 3.27)

project(test_entry_point VERSION 1.0.0 LANGUAGES CXX)
enable_testing()

add_executable(test_happy_path src/test_happy_path.cpp)
add_executable(test_timing src/test_timing.cpp)

add_subdirectory(lib/entry_point)
add_subdirectory(lib/asserter)

target_include_directories(test_happy_path SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib)
target_include_directories(test_timing SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib)

set_target_properties(test_happy_path test_timing PROPERTIES
    CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (EMSCRIPTEN)
    set_target_properties(test_happy_path test_timing PROPERTIES SUFFIX ".html")
endif()

add_test(
  NAME test_happy_path
  COMMAND test_happy_path
)

add_test(
  NAME test_timing
  COMMAND test_timing
)

