include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt6 COMPONENTS Core REQUIRED)

set(SOURCE_FILES
	measure.cpp
	profiler.cpp
)

set (MOC_HEADERS
)

set(HEADER_FILES
	measure.hpp
	profiler.hpp
    ${MOC_HEADERS}
)

add_library(MyLib STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

target_link_libraries(MyLib
    Qt6::Core
    coverage_config
	unwind
	unwind-ptrace
	unwind-generic
)
