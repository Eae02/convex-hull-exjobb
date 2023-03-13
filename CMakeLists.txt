cmake_minimum_required(VERSION 3.10)
project(ch)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal/CMakeLists.txt")
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/cgal/Installation/lib/cmake/CGAL)
	set(HAS_CGAL TRUE)
else()
	message(WARNING "cgal not found, cgal implementations will not be compiled")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/pcm/CMakeLists.txt")
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/pcm)
	set(HAS_PCM TRUE)
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

if (NO_AVX)
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*_avx.*\.cpp")
else()
	set_source_files_properties(src/implementations/jarvis_wrap_avx512.cpp PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512vl")	
	set_source_files_properties(src/implementations/jarvis_wrap_avx.cpp PROPERTIES COMPILE_FLAGS "-mavx")
	set_source_files_properties(src/implementations/quickhull_avx512.cpp PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512vl")
	set_source_files_properties(src/implementations/quickhull_avx2.cpp PROPERTIES COMPILE_FLAGS "-mavx -mavx2 -mfma")
endif()

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
	-Wno-missing-field-initializers
	-pedantic
	$<$<COMPILE_LANGUAGE:CXX>:-Wconversion-null>
	$<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(ch PRIVATE Boost::boost Threads::Threads)

if (${HAS_CGAL})
	find_package(CGAL REQUIRED COMPONENTS Core)
	target_link_libraries(ch PRIVATE CGAL::CGAL)
	target_compile_definitions(ch PRIVATE HAS_CGAL)
endif()

if (${HAS_PCM})
	target_link_libraries(ch PRIVATE PCM_STATIC_SILENT)
	target_include_directories(ch PRIVATE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/libs/pcm/src)
	target_compile_definitions(ch PRIVATE HAS_PCM)
endif()

if (TBB_PATH)
	target_link_libraries(ch PRIVATE ${TBB_PATH}/gnu_7.5_cxx11_64_relwithdebinfo/libtbb.so)
	target_include_directories(ch PRIVATE SYSTEM ${TBB_PATH}/include)
	target_compile_definitions(ch PRIVATE HAS_TBB)
else()
	find_package(TBB)
	if (${TBB_FOUND})
		target_link_libraries(ch PRIVATE TBB::tbb)
		target_compile_definitions(ch PRIVATE HAS_TBB)
	else()
		message(WARNING "tbb not found, parallel quickhull will not be compiled")
	endif()
endif()

set_target_properties(ch PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_NAME $<IF:$<CONFIG:Debug>,chd.bin,ch.bin>
	LINKER_LANGUAGE CXX
	LINK_OPTIONS $<$<CONFIG:Debug>:-fsanitize=address,undefined>
	CXX_STANDARD 20
)
