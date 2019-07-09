cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

find_package(Compsky REQUIRED asciify mysql)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


add_executable(rscraper-id2str "${SRC_DIR}/id2str.cpp")
target_compile_definitions(rscraper-id2str PRIVATE ID2STR_EXE)

add_executable(rscraper-str2id "${SRC_DIR}/id2str.cpp")
target_compile_definitions(rscraper-str2id PRIVATE STR2ID_EXE)


add_executable(rscraped-reason   "${SRC_DIR}/cmnts_from_subs_tagged.cpp" "${SRC_DIR}/id2str.cpp")
set_property(TARGET rscraped-reason PROPERTY CXX_STANDARD 17)
target_link_libraries(rscraped-reason ${WHICH_MYSQL_CLIENT} compsky_asciify compsky_mysql)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraped-reason z secur32 ssl crypto crypt32 ws2_32)
endif()

add_executable(rscraped-tagged-subs "${SRC_DIR}/cmnts_from_subs_tagged.cpp" "${SRC_DIR}/id2str.cpp")
set_property(TARGET rscraped-tagged-subs PROPERTY CXX_STANDARD 17)
target_link_libraries(rscraped-tagged-subs ${WHICH_MYSQL_CLIENT} compsky_asciify compsky_mysql)
target_compile_definitions(rscraped-tagged-subs PRIVATE SUB2TAG)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(rscraped-tagged-subs z secur32 ssl crypto crypt32 ws2_32)
endif()


install(
    TARGETS rscraper-id2str rscraper-str2id rscraped-reason rscraped-tagged-subs
    RUNTIME DESTINATION bin
        COMPONENT utils
)
