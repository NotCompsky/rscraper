cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Qt5Widgets REQUIRED)
find_package(Compsky REQUIRED asciify mysql)
set(CMAKE_AUTOMOC ON)


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

add_executable(
    rscraper-tags
    
    "${SRC_DIR}/main.cpp"
    
    "${SRC_DIR}/clbtn.cpp"
    "${SRC_DIR}/categorytab.cpp"
    "${SRC_DIR}/cltagswindow.cpp"
    "${SRC_DIR}/tagdialog.cpp"
)
target_include_directories(rscraper-tags PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-tags mysqlclient Qt5::Widgets compsky_asciify compsky_mysql)

install(
    TARGETS rscraper-tags
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
