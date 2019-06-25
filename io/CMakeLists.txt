cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

find_package(Compsky REQUIRED asciify mysql)

add_executable(rscraper-export "${CMAKE_CURRENT_SOURCE_DIR}/src/export.cpp")
target_include_directories(rscraper-export PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-export compsky_mysql compsky_asciify mysqlclient)
set_property(TARGET rscraper-export PROPERTY CXX_STANDARD 17)

add_executable(rscraper-import "${CMAKE_CURRENT_SOURCE_DIR}/src/import.cpp")
target_include_directories(rscraper-import PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-import compsky_mysql compsky_asciify mysqlclient)
set_property(TARGET rscraper-import PROPERTY CXX_STANDARD 17)

install(
    TARGETS rscraper-export rscraper-import
    RUNTIME DESTINATION bin
        COMPONENT utils
)
