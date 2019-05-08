cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
FIND_PACKAGE(Boost COMPONENTS regex REQUIRED)

set(CMAKE_BUILD_TYPE "Release")
include_directories(${CMAKE_ROOT}/inc)


add_executable(rscrape-cmnts src/scrape.cpp)
target_link_libraries(rscrape-cmnts curl)
target_link_libraries(rscrape-cmnts b64)
target_link_libraries(rscrape-cmnts boost::regex)
target_link_libraries(rscrape-cmnts mysqlcppconn)


add_executable(rscrape-mods src/scrape.cpp)
target_link_libraries(rscrape-cmnts curl)
target_link_libraries(rscrape-cmnts b64)
target_link_libraries(rscrape-cmnts mysqlcppconn)
target_compile_definitions(rscrape-cmnts PRIVATE SPIDER)


add_executable(srch-by-reason src/mysql__cmnts_from_subs_tagged.cpp)
target_link_libraries(rscrape-cmnts mysqlcppconn)


add_executable(srch-tagged-subs src/mysql__cmnts_from_subs_tagged.cpp)
target_link_libraries(rscrape-cmnts mysqlcppconn)
target_compile_definitions(rscrape-cmnts PRIVATE SUB2TAG)
