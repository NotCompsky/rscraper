cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

find_package(Qt5 COMPONENTS Widgets Charts)

if(NOT Qt5Widgets_FOUND)
    message(WARNING "Qt5::Widgets not found, so skipping compilation of rscraper-hub")
    return()
elseif(NOT Qt5Charts_FOUND)
    message(WARNING "Qt5::Charts not found, so skipping compilation of rscraper-hub")
    return()
endif()

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
    rscraper-hub
    
    "${SRC_DIR}/main.cpp"
    
    "${SRC_DIR}/mainwindow.cpp"
    
    # Main Tab
    "${SRC_DIR}/maintab.cpp"
    "${SRC_DIR}/wlbl_label.cpp"
    "${SRC_DIR}/wlbl_reasonwise_label.cpp"
    
    # Utils
    "${SRC_DIR}/name_dialog.cpp"
    
    # Messages
    "${SRC_DIR}/notfound.cpp"
    
    # 3rd party
    "${SRC_DIR}/3rdparty/donutbreakdownchart.cpp"
    "${SRC_DIR}/3rdparty/mainslice.cpp"
    
    # Tag Colour Picker tabs
    "${SRC_DIR}/categorytab.cpp"
    "${SRC_DIR}/clbtn.cpp"
    "${SRC_DIR}/tagnamelabel.cpp"
    "${SRC_DIR}/add_sub2tag_btn.cpp"
    "${SRC_DIR}/rm_tag_btn.cpp"
    
    "${SRC_DIR}/regex_editor.cpp"
    
    "${SRC_DIR}/cat_doughnut.cpp"
)
set_property(TARGET rscraper-hub PROPERTY CXX_STANDARD 17)
target_include_directories(rscraper-hub PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-hub mysqlclient Qt5::Widgets Qt5::Charts compsky_asciify compsky_mysql)

install(
    TARGETS rscraper-hub
    RUNTIME DESTINATION bin
        COMPONENT gui
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraper-hub secur32)
endif()
