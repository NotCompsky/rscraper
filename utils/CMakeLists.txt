cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Compsky REQUIRED asciify mysql)

add_executable(
    rscraper-init
    
    "${CMAKE_CURRENT_SOURCE_DIR}/init.cpp"
)
target_include_directories(rscraper-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-init mysqlclient compsky_mysql compsky_asciify)

install(
    TARGETS rscraper-init
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
