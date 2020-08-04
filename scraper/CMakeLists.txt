cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not


option(ENABLE_STATIC "Produce statically-linked executable" OFF)
option(BUILD_INIT_PROGRAM "Build rscraper-init-scraper" ON)
option(BUILD_SCRAPER "Build rscrape-cmnts" ON)


FIND_PACKAGE(Boost COMPONENTS regex)
FIND_PACKAGE(CURL)

if(ENABLE_STATIC)
	set(CURL_NAMES curl.a)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
else()
	set(CURL_NAMES curl)
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
endif()
find_library(CURL_LIB NAMES ${CURL_NAMES})
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})


if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set(MY_DEFINITIONS "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ID2STR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../utils/src")
set(SHARED_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared-src")

include_directories("${ID2STR_DIR}")
include_directories("${SHARED_SRC_DIR}")

if(NOT Boost_FOUND)
	message(WARNING "boost::regex not found, so there will be no regex matching")
endif()

set(SRC_FILES "${SRC_DIR}/filter_comment_body_regexp.cpp" "${SRC_DIR}/filter_comment_body.cpp" "${SRC_DIR}/filter_subreddit.cpp" "${SRC_DIR}/filter_user.cpp" "${SRC_DIR}/filter_init.cpp" "${SRC_DIR}/curl_utils.cpp" "${SRC_DIR}/redditcurl_utils.cpp")

set(BASE64_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/src/base64.c")
set(MY_TARGETS)

if(BUILD_INIT_PROGRAM)
	add_executable(rscraper-init-scraper "${SRC_DIR}/init.cpp")
	target_include_directories(rscraper-init-scraper PRIVATE ${COMPSKY_INCLUDE_DIRS})
	target_link_libraries(rscraper-init-scraper "${MYSQL_CLIENT}")
	list(APPEND MY_TARGETS "rscraper-init-scraper")
endif()

if(BUILD_SCRAPER)
	add_executable(rscrape-cmnts "${SRC_DIR}/scrape.cpp" "${BASE64_SRC}" ${SRC_FILES} "${ID2STR_DIR}/id2str.cpp" "${SHARED_SRC_DIR}/init_regexp_from_file")
	set_property(TARGET rscrape-cmnts PROPERTY CXX_STANDARD 17)
	target_link_libraries(rscrape-cmnts "${CURL_LIB}" "${MYSQL_CLIENT}")
	#target_link_libraries(rscrape-cmnts stdc++fs)
	if(Boost_FOUND)
		target_link_libraries(rscrape-cmnts ${Boost_REGEX_LIBRARY})
		target_compile_definitions(rscrape-cmnts PRIVATE USE_BOOST_REGEX)
	endif()
	list(APPEND MY_TARGETS "rscrape-cmnts")
endif()

foreach(tgt ${MY_TARGETS})
	target_compile_definitions("${tgt}" PRIVATE "${MY_DEFINITIONS}")
endforeach(tgt)

include(GNUInstallDirs)
install(
	TARGETS ${MY_TARGETS}
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT scraper
)
