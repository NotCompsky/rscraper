cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

find_package(Compsky REQUIRED asciify mysql)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


add_executable(rscraped-reason   "${SRC_DIR}/cmnts_from_subs_tagged.cpp")
set_property(TARGET rscraped-reason PROPERTY CXX_STANDARD 17)
target_link_libraries(rscraped-reason mysqlclient compsky_asciify compsky_mysql)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraped-reason z secur32 ssl crypto crypt32 ws2_32)
endif()

add_executable(rscraped-tagged-subs "${SRC_DIR}/cmnts_from_subs_tagged.cpp")
set_property(TARGET rscraped-tagged-subs PROPERTY CXX_STANDARD 17)
target_link_libraries(rscraped-tagged-subs mysqlclient compsky_asciify compsky_mysql)
target_compile_definitions(rscraped-tagged-subs PRIVATE SUB2TAG)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraped-tagged-subs z secur32 ssl crypto crypt32 ws2_32)
endif()


install(
    TARGETS rscraped-reason rscraped-tagged-subs
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
