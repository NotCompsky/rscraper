cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()


FIND_PACKAGE(Boost COMPONENTS regex)
FIND_PACKAGE(CURL)


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


if(IS_DIRECTORY "C:/Program Files (x86)/CURL")
    include_directories("C:/Program Files (x86)/CURL/include")
endif()


if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cmake/FindMYSQL.cmake")
    include_directories("${MYSQL_INCLUDE_DIR}")
    link_directories("${MYSQL_LIBRARIES}")
endif()

message(STATUS "COMPSKY_INCLUDE_DIRS: ${COMPSKY_INCLUDE_DIRS}")
include_directories("C:/Program Files/Compsky/include") # Should have been defined by COMPSKY_INCLUDE_DIRS
link_directories("C:/Program Files/Compsky/bin")


if("${MY_TARGETS} " STREQUAL " ")
    set(MY_TARGETS "rscrape-cmnts" "rscrape-mods" "rscraper-init-scraper")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


if(NOT Boost_FOUND)
    message(WARNING "boost::regex not found, so there will be no regex matching in the examples")
endif()

# Rename example files
set(SRC_FILES "${SRC_DIR}/filter_comment_body_regexp.cpp" "${SRC_DIR}/filter_comment_body.cpp" "${SRC_DIR}/filter_subreddit.cpp" "${SRC_DIR}/filter_user.cpp" "${SRC_DIR}/filter_init.cpp" "${SRC_DIR}/curl_utils.cpp" "${SRC_DIR}/redditcurl_utils.cpp" "${SRC_DIR}/reddit_utils.cpp")

set(BASE64_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/src/base64.c")

if("rscraper-init-scraper" IN_LIST MY_TARGETS)
    add_executable(rscraper-init-scraper "${SRC_DIR}/init.cpp")
    target_include_directories(rscraper-init-scraper PRIVATE ${COMPSKY_INCLUDE_DIRS})
endif()

if("rscrape-cmnts" IN_LIST MY_TARGETS)
    add_executable(rscrape-cmnts "${SRC_DIR}/scrape.cpp" "${BASE64_SRC}" ${SRC_FILES})
    set_property(TARGET rscrape-cmnts PROPERTY CXX_STANDARD 17)
    if(${FS_EXPERIMENTAL})
        target_compile_definitions(rscrape-cmnts PRIVATE FS_EXPERIMENTAL=1)
        target_link_libraries(rscrape-cmnts stdc++fs)
    endif()
    target_link_libraries(rscrape-cmnts curl)
    target_link_libraries(rscrape-cmnts compsky_mysql)
    target_link_libraries(rscrape-cmnts mysqlclient)
    target_link_libraries(rscrape-cmnts compsky_asciify)
    #target_link_libraries(rscrape-cmnts stdc++fs)
    if(Boost_FOUND)
        target_link_libraries(rscrape-cmnts ${Boost_REGEX_LIBRARY})
        target_compile_definitions(rscrape-cmnts PRIVATE USE_BOOST_REGEX)
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_compile_definitions(rscrape-cmnts PUBLIC CURL_STATICLIB)
        target_link_libraries(rscrape-cmnts ssh2 gnutls idn2 nettle hogweed gcrypt gpg-error unistring iconv ssl crypto z ws2_32 crypt32 wldap32 gmp pthread)
    endif()
endif()
if("rscrape-mods" IN_LIST MY_TARGETS)
    add_executable(rscrape-mods "${SRC_DIR}/getmods.cpp" "${BASE64_SRC}" ${SRC_FILES})
    set_property(TARGET rscrape-mods PROPERTY CXX_STANDARD 17)
    if(${FS_EXPERIMENTAL})
        target_compile_definitions(rscrape-mods PRIVATE FS_EXPERIMENTAL=1)
        target_link_libraries(rscrape-mods stdc++fs)
    endif()
    target_link_libraries(rscrape-mods curl)
    target_link_libraries(rscrape-mods compsky_mysql)
    target_link_libraries(rscrape-mods mysqlclient)
    target_link_libraries(rscrape-mods compsky_asciify)
    target_compile_definitions(rscrape-mods PRIVATE SPIDER)
    if(Boost_FOUND)
        target_link_libraries(rscrape-mods ${Boost_REGEX_LIBRARY})
        target_compile_definitions(rscrape-mods PRIVATE USE_BOOST_REGEX)
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_compile_definitions(rscrape-mods PUBLIC CURL_STATICLIB)
        target_link_libraries(rscrape-mods ssh2 gnutls idn2 nettle hogweed gcrypt gpg-error unistring iconv ssl crypto z ws2_32 crypt32 wldap32 gmp pthread)
    endif()
endif()

foreach(tgt ${MY_TARGETS})
    target_compile_definitions("${tgt}" PRIVATE "${MY_DEFINITIONS}")
endforeach(tgt)


install(
    TARGETS ${MY_TARGETS}
    RUNTIME DESTINATION bin
        COMPONENT scraper
)
