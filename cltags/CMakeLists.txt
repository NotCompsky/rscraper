cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE "Release")

add_executable(cltags src/main.cpp src/cltagswindow.cpp src/cltagswindow.h)
target_link_libraries(cltags Qt5::Widgets)
target_link_libraries(cltags mysqlcppconn)
