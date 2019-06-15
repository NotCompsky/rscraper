cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not
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



# For Windows #
if(IS_DIRECTORY "C:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "C:/Program Files/MySQL/MySQL Server 8.0")
elseif(IS_DIRECTORY "D:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "D:/Program Files/MySQL/MySQL Server 8.0")
elseif(IS_DIRECTORY "E:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "E:/Program Files/MySQL/MySQL Server 8.0")
elseif(IS_DIRECTORY "F:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "F:/Program Files/MySQL/MySQL Server 8.0")
elseif(IS_DIRECTORY "G:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "G:/Program Files/MySQL/MySQL Server 8.0")
elseif(IS_DIRECTORY "H:/Program Files/MySQL/MySQL Server 8.0")
    set(WIN_MYSQL_DIR "H:/Program Files/MySQL/MySQL Server 8.0")
endif()
if(NOT "${WIN_MYSQL_DIR} " STREQUAL " ") # Trailing space is important - string equality check fails for two empty strings
    include_directories("${WIN_MYSQL_DIR}/include")
    link_directories("${WIN_MYSQL_DIR}/lib")
    message(STATUS "Using MySQL directory: ${WIN_MYSQL_DIR}")
endif()
message(STATUS "COMPSKY_INCLUDE_DIRS: ${COMPSKY_INCLUDE_DIRS}")
include_directories("C:/Program Files/Compsky/include") # Should have been defined by COMPSKY_INCLUDE_DIRS
link_directories("C:/Program Files/Compsky/bin")


if("${MY_TARGETS} " STREQUAL " ")
    set(MY_TARGETS "rscraper-id2str" "rscraper-str2id" "rscrape-cmnts" "rscrape-mods" "rscraped-reason" "rscraped-tagged-subs")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


if(Boost_FOUND)
    set(USE_BOOST_REGEX 1)
else()
    message(WARNING "boost::regex not found, so there will be no regex matching in the examples")
endif()

# Rename example files
foreach(infile "${SRC_DIR}/filter_comment_body_regexp.cpp" "${SRC_DIR}/filter_comment_body.cpp" "${SRC_DIR}/filter_subreddit.cpp" "${SRC_DIR}/filter_user.cpp")
    if(NOT EXISTS "${infile}")
        configure_file("${infile}.in" "${infile}")
    endif()
endforeach()

if("rscraper-id2str" IN_LIST MY_TARGETS)
    add_executable(rscraper-id2str "${SRC_DIR}/id2str.c")
    target_compile_definitions(rscraper-id2str PRIVATE TOSTR)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(rscraper-id2str z secur32 ssl crypto crypt32 ws2_32)
    endif()
endif()

if("rscraper-str2id" IN_LIST MY_TARGETS)
    add_executable(rscraper-str2id "${SRC_DIR}/id2str.c")
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(rscraper-str2id z secur32 ssl crypto crypt32 ws2_32)
    endif()
endif()
if("rscrape-cmnts" IN_LIST MY_TARGETS)
    add_executable(rscrape-cmnts "${SRC_DIR}/scrape.cpp")
    set_property(TARGET rscrape-cmnts PROPERTY CXX_STANDARD 17)
    target_link_libraries(rscrape-cmnts curl)
    target_link_libraries(rscrape-cmnts b64)
    target_link_libraries(rscrape-cmnts ${Boost_REGEX_LIBRARY})
    target_link_libraries(rscrape-cmnts mysqlclient)
    target_link_libraries(rscrape-cmnts compsky_asciify)
    target_link_libraries(rscrape-cmnts compsky_mysql)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_compile_definitions(rscrape-cmnts PUBLIC CURL_STATICLIB)
    #    target_link_libraries(rscrape-cmnts idn2 ssh2 unistring gcrypt z secur32 ssl crypto crypt32 ws2_32) # gmp gnurx unistring  nettle gnutls gcrypt
    endif()
endif()
if("rscrape-mods" IN_LIST MY_TARGETS)
    add_executable(rscrape-mods "${SRC_DIR}/scrape.cpp")
    set_property(TARGET rscrape-mods PROPERTY CXX_STANDARD 17)
    target_link_libraries(rscrape-mods curl)
    target_link_libraries(rscrape-mods b64)
    target_link_libraries(rscrape-mods ${Boost_REGEX_LIBRARY})
    target_link_libraries(rscrape-mods mysqlclient)
    target_compile_definitions(rscrape-mods PRIVATE SPIDER)
    target_link_libraries(rscrape-mods compsky_asciify)
    target_link_libraries(rscrape-mods compsky_mysql)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_compile_definitions(rscrape-mods PUBLIC CURL_STATICLIB)
        target_link_libraries(rscrape-mods ssh2)
        #target_link_libraries(rscrape-mods ssh2 idn2 gnutls unistring nettle  z secur32 ssl crypto crypt32 ws2_32) # idn2 ssh2 unistring gcrypt  # gmp gnurx unistring  nettle gnutls gcrypt
    endif()
endif()
if("rscraped-reason" IN_LIST MY_TARGETS)
    add_executable(rscraped-reason   "${SRC_DIR}/mysql__cmnts_from_subs_tagged.cpp")
    set_property(TARGET rscraped-reason PROPERTY CXX_STANDARD 17)
    target_link_libraries(rscraped-reason mysqlclient compsky_asciify compsky_mysql)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(rscraped-reason z secur32 ssl crypto crypt32 ws2_32)
    endif()
endif()
if("rscraped-tagged-subs" IN_LIST MY_TARGETS)
    add_executable(rscraped-tagged-subs "${SRC_DIR}/mysql__cmnts_from_subs_tagged.cpp")
    set_property(TARGET rscraped-tagged-subs PROPERTY CXX_STANDARD 17)
    target_link_libraries(rscraped-tagged-subs mysqlclient compsky_asciify compsky_mysql)
    target_compile_definitions(rscraped-tagged-subs PRIVATE SUB2TAG)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        target_link_libraries(rscraped-tagged-subs z secur32 ssl crypto crypt32 ws2_32)
    endif()
endif()

foreach(tgt ${MY_TARGETS})
    foreach(dfn ${MY_DEFINITIONS})
        target_compile_definitions("${tgt}" PRIVATE "${dfn}")
    endforeach(dfn)
endforeach(tgt)


install(
    TARGETS ${MY_TARGETS}
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        COMPONENT bin
)
