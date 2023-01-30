cmake_minimum_required(VERSION 3.16)
project(ch)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal/CMakeLists.txt")
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal/Installation/lib/cmake/CGAL)
	set(HAS_CGAL TRUE)
else()
	message(WARNING "cgal not found, cgal implementations will not be compiled")
endif()

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

find_package(Boost REQUIRED)
target_link_libraries(ch PRIVATE Boost::boost)

if (${HAS_CGAL})
	find_package(CGAL REQUIRED COMPONENTS Core)
	target_link_libraries(ch PRIVATE CGAL::CGAL)
	target_compile_definitions(ch PRIVATE HAS_CGAL)
endif()

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
