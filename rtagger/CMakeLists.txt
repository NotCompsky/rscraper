cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cmake/GolangSimple.cmake")

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
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(rscraper-tagger SHARED "${SRC_DIR}/rtagger.cpp")
target_link_libraries(rscraper-tagger mysqlclient compsky_asciify compsky_mysql)
set_property(TARGET rscraper-tagger PROPERTY CXX_STANDARD 17)

ADD_GO_INSTALLABLE_PROGRAM(rscraper-tagger-server "${SRC_DIR}/server.go")

foreach(tgt "rscraper-tagger")
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(tgt)

set_target_properties(rscraper-tagger PROPERTIES PUBLIC_HEADER "${INC_DIR}/rtagger.hpp")

install(
    TARGETS rscraper-tagger
    EXPORT RScraperTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/rscraper"
        COMPONENT dev
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraper-tagger z secur32 ssl crypto crypt32 ws2_32)
endif()
