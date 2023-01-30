cmake_minimum_required(VERSION 3.16)
project(ch)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal)
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal/Installation/lib/cmake/CGAL)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(ch ${SOURCE_FILES})

target_compile_options(ch PRIVATE
	-Wall
	-Wextra
	-Wshadow
	-Wcast-qual
	-Wfloat-conversion
	-Woverlength-strings
	-Wpointer-arith
	-Wunused-local-typedefs
	-Wunused-result
	-Wvarargs
	-Wvla
	-Wstrict-aliasing
	-Wwrite-strings
	-pedantic
	$<$<COMPILE_LANGUAGE:CXX>:-Wconversion-null>
	$<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_link_options(ch PRIVATE
	$<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

find_package(CGAL REQUIRED COMPONENTS Core)
find_package(Boost REQUIRED)

target_link_libraries(ch PRIVATE CGAL::CGAL Boost::boost)

find_package(TBB)
if (${TBB_FOUND})
	target_link_libraries(ch PRIVATE TBB::tbb)
	target_compile_definitions(ch PRIVATE HAS_TBB)
else()
	message(WARNING "tbb not found, parallel quickhull will not be compiled")
endif()

set_target_properties(ch PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_NAME $<IF:$<CONFIG:Debug>,chd.bin,ch.bin>
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
)
