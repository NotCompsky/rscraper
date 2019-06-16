cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
find_package(Compsky REQUIRED asciify mysql)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


add_executable(rscraper-init "${SRC_DIR}/init.cpp")
target_include_directories(rscraper-init PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-init compsky_mysql_create_config compsky_asciify)
set_property(TARGET rscraper-init PROPERTY CXX_STANDARD 17)


add_executable(rscraper-id2str "${SRC_DIR}/id2str.c")
target_compile_definitions(rscraper-id2str PRIVATE TOSTR)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraper-id2str z secur32 ssl crypto crypt32 ws2_32)
endif()


add_executable(rscraper-str2id "${SRC_DIR}/id2str.c")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraper-str2id z secur32 ssl crypto crypt32 ws2_32)
endif()



install(
    TARGETS rscraper-init rscraper-id2str rscraper-str2id
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
