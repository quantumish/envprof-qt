include_directories(${CMAKE_CURRENT_BINARY_DIR} /opt/cuda/targets/x86_64-linux/include/)

find_package(Qt6 COMPONENTS Core REQUIRED)

set(SOURCE_FILES
    measure.cpp
	parser.cpp
	profiler.cpp
)

set (MOC_HEADERS
)

set(HEADER_FILES
    measure.hpp
	parser.hpp
	profiler.hpp
    ${MOC_HEADERS}
)

add_library(envprof STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

target_link_libraries(envprof
    Qt6::Core
    coverage_config
	unwind
	unwind-ptrace
	unwind-generic
	nvidia-ml
)
