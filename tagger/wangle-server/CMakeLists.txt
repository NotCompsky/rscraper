cmake_minimum_required(VERSION 3.7)

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

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type")
option(MANGLE_JS_NAMES "Minimise the names of JavaScript functions and variables" OFF)
option(DISPLAY_LICENSES_INLINE "Display static component licenses inline" ON)
option(ENABLE_STATIC "Build static executable, rather than linked" OFF)


if(MANGLE_JS_NAMES)
	set(MANGLE_JS_NAMES_ARGS "--mangle")
else()
	set(MANGLE_JS_NAMES_ARGS "")
endif()

find_package(fizz REQUIRED)
find_package(Threads REQUIRED)
find_package(folly REQUIRED)
find_package(wangle REQUIRED)
find_package(gflags REQUIRED)
find_package(Compsky REQUIRED)

if(ENABLE_STATIC)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
else()
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
endif()
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(AUTO_SRC_DIR "${SRC_DIR}/auto-generated")
set(TAGGER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(TAGGER_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")


file(MAKE_DIRECTORY "${AUTO_SRC_DIR}")
# Creates directory during build system generation



add_executable(server "${SRC_DIR}/server.cpp" "${TAGGER_SRC_DIR}/tagger.cpp"
)
set_target_properties(
	server
	PROPERTIES
		CXX_STANDARD 17
		INTERPROCEDURAL_OPTIMIZATION TRUE
		LINK_FLAGS_RELEASE -s
)
target_include_directories(server PRIVATE "${WANGLE_INCLUDE_DIR}" "${COMPSKY_INCLUDE_DIRS}" "${TAGGER_INC_DIR}"
)
target_link_libraries(server PRIVATE "${MYSQL_CLIENT}" wangle::wangle )



add_custom_target(
	auto__server_determine_response_hpp ALL
	DEPENDS "${SRC_DIR}/server-endpoints.txt" # CMake does not seem to have an option to only rerun when dependency is modified
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/triegen.py" "${SRC_DIR}/server-endpoints.txt" "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	BYPRODUCTS "${AUTO_SRC_DIR}/auto__server-determine-response.hpp"
	COMMENT "Generating bigboi switch statement"
)

add_custom_target(
	js_hpp ALL
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/minjs.py" "${MANGLE_JS_NAMES_ARGS}" "ALL" "${AUTO_SRC_DIR}/js.hpp" "${SRC_DIR}/static/js/" #"${SRC_DIR}/static/js/3rd-party/datetimepicker.js"
	BYPRODUCTS "${AUTO_SRC_DIR}/js.hpp"
	COMMENT "Generating minimised JavaScript header"
)

add_custom_target(
	svg_hpp ALL
	COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_DIR}/svggen.py" "${DISPLAY_LICENSES_INLINE_ARGS}" "${AUTO_SRC_DIR}/svg.hpp" "${SRC_DIR}/html/svg"
	BYPRODUCTS "${AUTO_SRC_DIR}/svg.hpp"
	COMMENT "Generating SVG icons header"
)





set(DISPLAY_LICENSES_INLINE_ARGS)
if(DISPLAY_LICENSES_INLINE)
	set(DISPLAY_LICENSES_INLINE_ARGS "--display-licences")
endif()

foreach(var NULL_REASONS;NULL_TAGS;NULL_USERTAGS)
    if(${var})
        message(STATUS "${var} = ${${var}}")
        target_compile_definitions(server PRIVATE "${var}")
    endif()
endforeach()
if(n_cached)
    target_compile_definitions(server PRIVATE "n_cached=${n_cached}")
endif()


add_dependencies(server auto__server_determine_response_hpp js_hpp svg_hpp)
