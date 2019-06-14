cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
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

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(b64 STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cencode.c")

set_target_properties(b64 PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/b64/cencode.h")

install(
    TARGETS b64
    # Adds the mysql library to the "export-set"
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT shlib
    PUBLIC_HEADER DESTINATION "include/b64"
        COMPONENT dev
)
