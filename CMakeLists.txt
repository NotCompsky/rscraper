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


if ("${MY_TARGETS} " STREQUAL " ") # Extra character necessary - empty string comparison doesn't work
    # e.g. cmake .. -DMY_TARGETS=txt;img
    set(MY_TARGETS "id2str" "str2id" "rscrape-cmnts" "rscrape-mods" "srch-by-reason" "srch-tagged-subs")
endif()

include_directories("./include")


if ("id2str" IN_LIST MY_TARGETS)
    add_executable(id2str src/id2str.c)
    target_compile_definitions(id2str PRIVATE TOSTR)
endif()
if ("str2id" IN_LIST MY_TARGETS)
    add_executable(str2id src/id2str.c)
endif()

if ("rscrape-cmnts" IN_LIST MY_TARGETS)
    add_executable(rscrape-cmnts src/scrape.cpp)
    target_link_libraries(rscrape-cmnts curl)
    target_link_libraries(rscrape-cmnts b64)
    target_link_libraries(rscrape-cmnts ${Boost_REGEX_LIBRARY})
    target_link_libraries(rscrape-cmnts mysqlclient)
    target_link_libraries(rscrape-cmnts compsky_asciify)
    target_link_libraries(rscrape-cmnts compsky_mysql)
endif()
if ("rscrape-mods" IN_LIST MY_TARGETS)
    add_executable(rscrape-mods src/scrape.cpp)
    target_link_libraries(rscrape-mods curl)
    target_link_libraries(rscrape-mods b64)
    target_link_libraries(rscrape-mods ${Boost_REGEX_LIBRARY})
    target_link_libraries(rscrape-mods mysqlclient)
    target_compile_definitions(rscrape-mods PRIVATE SPIDER)
    target_link_libraries(rscrape-mods compsky_asciify)
    target_link_libraries(rscrape-mods compsky_mysql)
endif()

if ("srch-by-reason" IN_LIST MY_TARGETS)
    add_executable(srch-by-reason   src/mysql__cmnts_from_subs_tagged.cpp)
    target_link_libraries(srch-by-reason mysqlclient compsky_asciify compsky_mysql)
endif()
if ("srch-tagged-subs" IN_LIST MY_TARGETS)
    add_executable(srch-tagged-subs src/mysql__cmnts_from_subs_tagged.cpp)
    target_link_libraries(srch-tagged-subs mysqlclient compsky_asciify compsky_mysql)
    target_compile_definitions(srch-tagged-subs PRIVATE SUB2TAG)
endif()



foreach(tgt ${MY_TARGETS})
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(tgt)
