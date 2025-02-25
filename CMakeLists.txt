project(entry_point)

add_library(entry_point INTERFACE
    entry_point.hpp
)

set_target_properties(entry_point PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
    LINKER_LANGUAGE CXX
)
