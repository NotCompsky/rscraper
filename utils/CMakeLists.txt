cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


add_executable(rscraper-id2str "${SRC_DIR}/id2str.cpp")
target_compile_definitions(rscraper-id2str PRIVATE ID2STR_EXE)


add_executable(rscraper-str2id "${SRC_DIR}/id2str.cpp")
target_compile_definitions(rscraper-str2id PRIVATE STR2ID_EXE)


install(
    TARGETS rscraper-id2str rscraper-str2id
    RUNTIME DESTINATION bin
        COMPONENT utils
)
