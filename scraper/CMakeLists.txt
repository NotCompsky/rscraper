cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
FIND_PACKAGE(Compsky COMPONENTS asciify mysql REQUIRED)
FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)


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


set(MY_TARGETS "rscraper-id2str" "rscraper-str2id" "rscrape-cmnts" "rscrape-mods" "rscraped-reason" "rscraped-tagged-subs")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


add_executable(rscraper-id2str "${SRC_DIR}/id2str.c")
target_compile_definitions(rscraper-id2str PRIVATE TOSTR)

add_executable(rscraper-str2id "${SRC_DIR}/id2str.c")

add_executable(rscrape-cmnts "${SRC_DIR}/scrape.cpp")
target_link_libraries(rscrape-cmnts curl)
target_link_libraries(rscrape-cmnts b64)
target_link_libraries(rscrape-cmnts ${Boost_REGEX_LIBRARY})
target_link_libraries(rscrape-cmnts mysqlclient)
target_link_libraries(rscrape-cmnts compsky_asciify)
target_link_libraries(rscrape-cmnts compsky_mysql)

add_executable(rscrape-mods "${SRC_DIR}/scrape.cpp")
target_link_libraries(rscrape-mods curl)
target_link_libraries(rscrape-mods b64)
target_link_libraries(rscrape-mods ${Boost_REGEX_LIBRARY})
target_link_libraries(rscrape-mods mysqlclient)
target_compile_definitions(rscrape-mods PRIVATE SPIDER)
target_link_libraries(rscrape-mods compsky_asciify)
target_link_libraries(rscrape-mods compsky_mysql)

add_executable(rscraped-reason   "${SRC_DIR}/mysql__cmnts_from_subs_tagged.cpp")
target_link_libraries(rscraped-reason mysqlclient compsky_asciify compsky_mysql)

add_executable(rscraped-tagged-subs "${SRC_DIR}/mysql__cmnts_from_subs_tagged.cpp")
target_link_libraries(rscraped-tagged-subs mysqlclient compsky_asciify compsky_mysql)
target_compile_definitions(rscraped-tagged-subs PRIVATE SUB2TAG)


foreach(tgt ${MY_TARGETS})
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(tgt)


install(
    TARGETS rscraper-id2str rscraper-str2id rscrape-cmnts rscrape-mods rscraped-reason rscraped-tagged-subs
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
