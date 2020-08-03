cmake_minimum_required(VERSION 3.13)

# If you have installed Proxygen, you can use
# cmake -Dproxygen_root_DIR=/path/to/proxygen/root/dir

if(proxygen_root_DIR)
	set(proxygen_cmake_DIR "${proxygen_root_DIR}/proxygen/_build/deps/lib/cmake")
	
	if(NOT wangle_DIR)
		set(wangle_DIR "${proxygen_cmake_DIR}/wangle")
	endif()
	
	if(NOT folly_DIR)
		set(folly_DIR "${proxygen_cmake_DIR}/folly")
	endif()
	
	if(NOT fizz_DIR)
		set(fizz_DIR "${proxygen_cmake_DIR}/fizz")
	endif()
else()
	message(WARNING "You might want to define -Dproxygen_root_DIR=/path/to/proxygen/root for ease - defines proxygen_cmake_DIR, wangle_DIR, folly_DIR and fizz_DIR")
endif()

project(RScraperWangle C CXX)

find_package(fizz REQUIRED)
find_package(Threads REQUIRED)
find_package(folly REQUIRED)
find_package(wangle REQUIRED)
find_package(gflags REQUIRED)
find_package(Compsky REQUIRED)

if(ENABLE_STATIC)
	set(MYSQLCLIENT_LIB_NAMES
		mariadbclient.a
		libmariadbclient.a
		mariadb/mariadbclient.a
		mariadb/libmariadbclient.a
		mysqlclient.a
		libmysqlclient.a
		mysql/mysqlclient.a
		mysql/libmysqlclient.a
	)
else()
	set(MYSQLCLIENT_LIB_NAMES mariadbclient mysqlclient)
endif()
find_library(MYSQL_CLIENT NAMES ${MYSQLCLIENT_LIB_NAMES})

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TAGGER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(TAGGER_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

add_executable(server "${SRC_DIR}/server.cpp" "${SRC_DIR}/FrameDecoder.cpp" "${TAGGER_SRC_DIR}/tagger.cpp")
set_target_properties(
	server
	PROPERTIES
		CXX_STANDARD 17
		INTERPROCEDURAL_OPTIMIZATION TRUE
		LINK_FLAGS_RELEASE -s
)
target_include_directories(server PRIVATE "${WANGLE_INCLUDE_DIR}" "${COMPSKY_INCLUDE_DIRS}" "${TAGGER_INC_DIR}")
target_link_libraries(server wangle::wangle "${MYSQL_CLIENT}")
foreach(var NULL_REASONS;NULL_TAGS;NULL_USERTAGS)
	if(${var})
		message(STATUS "${var} = ${${var}}")
		target_compile_definitions(server PRIVATE "${var}")
	endif()
endforeach()
if(n_cached)
	target_compile_definitions(server PRIVATE "n_cached=${n_cached}")
endif()
