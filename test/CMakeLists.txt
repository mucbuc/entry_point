cmake_minimum_required(VERSION 3.27)

project(Example)

add_subdirectory(lib/entry_point)
add_executable(Example src/test.cpp)

set_target_properties(Example PROPERTIES
    CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (EMSCRIPTEN)
    set_target_properties(Example PROPERTIES SUFFIX ".html")
endif()
