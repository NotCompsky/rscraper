cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

find_package(Compsky REQUIRED mysql)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(MY_DEFINITIONS "DEBUG=1")
else()
	set(MY_DEFINITIONS)
endif()


add_executable(rscraper-annoying "${SRC_DIR}/annoying.cpp")
set_property(TARGET rscraper-annoying PROPERTY CXX_STANDARD 17)
target_include_directories(rscraper-annoying PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/include")
target_link_libraries(rscraper-annoying compsky_mysql)
target_compile_definitions(rscraper-annoying PRIVATE "${MY_DEFINITIONS}")

install(
	TARGETS rscraper-annoying
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT annoying
)
