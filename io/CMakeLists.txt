cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Compsky REQUIRED asciify mysql)

add_executable(rscraper-export "${CMAKE_CURRENT_SOURCE_DIR}/src/export.cpp")
target_include_directories(rscraper-export PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-export compsky_mysql compsky_asciify mysqlclient)
set_property(TARGET rscraper-export PROPERTY CXX_STANDARD 17)

install(
    TARGETS rscraper-export
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
