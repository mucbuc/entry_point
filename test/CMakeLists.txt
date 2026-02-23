cmake_minimum_required(VERSION 3.27)

project(Example)

add_subdirectory(lib/entry_point)
add_subdirectory(lib/asserter)

add_executable(Example src/test.cpp)
target_include_directories(Example SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/lib)

set_target_properties(Example PROPERTIES
    CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (EMSCRIPTEN)
    set_target_properties(Example PROPERTIES SUFFIX ".html")
endif()
