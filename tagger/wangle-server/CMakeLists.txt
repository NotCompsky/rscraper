cmake_minimum_required(VERSION 3.13)

# If you have installed Proxygen, you can use
# cmake -Dproxygen_root_DIR=/path/to/proxygen/root/dir

if(proxygen_root_DIR)
	message(STATUS "proxygen_root_DIR\t${proxygen_root_DIR}")
	
	set(proxygen_cmake_DIR "${proxygen_root_DIR}/proxygen/_build/deps/lib/cmake")
	
	if(NOT wangle_DIR)
		set(wangle_DIR "${proxygen_cmake_DIR}/wangle")
		message(STATUS "wangle_DIR\t${wangle_DIR}")
	endif()
	
	if(NOT folly_DIR)
		set(folly_DIR "${proxygen_cmake_DIR}/folly")
		message(STATUS "folly_DIR\t${folly_DIR}")
	endif()
	
	if(NOT fizz_DIR)
		set(fizz_DIR "${proxygen_cmake_DIR}/fizz")
		message(STATUS "fizz_DIR\t${fizz_DIR}")
	endif()
endif()

project(RScraperWangle C CXX)

find_package(fizz REQUIRED)
find_package(Threads REQUIRED)
find_package(folly REQUIRED)
find_package(wangle REQUIRED)
find_package(gflags REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(server "${SRC_DIR}/server.cpp" "${SRC_DIR}/FrameDecoder.cpp")
set_property(TARGET server PROPERTY CXX_STANDARD 17)
target_include_directories(server PRIVATE "${WANGLE_INCLUDE_DIR}")
target_link_libraries(server wangle::wangle)
