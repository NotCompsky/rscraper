cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

FIND_PACKAGE(Compsky COMPONENTS asciify mysql REQUIRED)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug
	set(MY_DEFINITIONS "DEBUG")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
	endif()
else()
	set(CMAKE_BUILD_TYPE "Release")
	set(MY_DEFINITIONS "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories("${INC_DIR}")

add_library(rscraper-tagger SHARED "${SRC_DIR}/tagger.cpp")
target_link_libraries(rscraper-tagger ${WHICH_MYSQL_CLIENT} compsky_asciify compsky_mysql)
set_property(TARGET rscraper-tagger PROPERTY CXX_STANDARD 17)




add_custom_target(rscraper-tagger-server ALL DEPENDS rscraper-tagger)
add_custom_command(
	TARGET rscraper-tagger-server
	COMMAND
		go build
		-o "${CMAKE_CURRENT_BINARY_DIR}/rscraper-tagger-server"
		"${SRC_DIR}/server.go"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/rscraper-tagger-server"
	DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT tagger
)




target_compile_definitions(rscraper-tagger PRIVATE ${MY_DEFINITIONS})

set_target_properties(rscraper-tagger PROPERTIES PUBLIC_HEADER "${INC_DIR}/rscraper/tagger.hpp")


install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/rscraper-tagger-server.py" DESTINATION "${CMAKE_INSTALL_BINDIR}")


install(
	TARGETS rscraper-tagger
	EXPORT RScraperTargets
	PUBLIC_HEADER DESTINATION
		"${INSTALL_INCLUDE_DIR}/rscraper"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT tagger
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(rscraper-tagger z secur32 ssl crypto crypt32 ws2_32)
endif()
