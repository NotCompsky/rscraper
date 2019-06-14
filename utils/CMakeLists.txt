cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Compsky REQUIRED asciify mysql)

add_executable(
    rscraper-init
    
    "${CMAKE_CURRENT_SOURCE_DIR}/init.cpp"
)
target_include_directories(rscraper-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-init compsky_mysql_create_config compsky_asciify)
set_property(TARGET rscraper-init PROPERTY CXX_STANDARD 17)

install(
    TARGETS rscraper-init
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
