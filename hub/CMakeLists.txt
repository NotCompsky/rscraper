cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

find_package(Qt5 COMPONENTS Widgets Charts)
find_package(Exig)


if(NOT Exig_FOUND)
	message(WARNING "Skipping rscraper-hub, as exig was not found")
	return()
endif()

if(NOT Qt5_FOUND)
	message(WARNING "Skipping rscraper-gui, as Qt5::Widgets and/or Qt5::Charts was not found")
	return()
endif()


find_package(Boost COMPONENTS regex)


if(NOT Qt5Widgets_FOUND)
	message(WARNING "Qt5::Widgets not found, so skipping compilation of rscraper-hub")
	return()
elseif(NOT Qt5Charts_FOUND)
	message(WARNING "Qt5::Charts not found, so skipping compilation of rscraper-hub")
	return()
endif()

find_package(Compsky REQUIRED)


if(ENABLE_STATIC)
	set(MYSQL_CLIENT_NAMES mariadbclient.a libmariadbclient.a mariadb/mariadbclient.a mariadb/libmariadbclient.a mysqlclient.a libmysqlclient.a mysql/mysqlclient.a mysql/libmysqlclient.a)
else()
	set(MYSQL_CLIENT_NAMES mariadbclient mysqlclient)
endif()
find_library(MYSQL_CLIENT NAMES ${MYSQL_CLIENT_NAMES})


set(CMAKE_AUTOMOC ON)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(UTILS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../utils/src")
set(SHARED_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared-src")

include_directories("${UTILS_SRC_DIR}" "${SHARED_SRC_DIR}")

add_executable(
	rscraper-hub
	
	"${SRC_DIR}/main.cpp"
	
	"${SRC_DIR}/mainwindow.cpp"
	
	# Other Tabs
	"${SRC_DIR}/main_tab.cpp"
	
	"${SRC_DIR}/scraper_tab.cpp"
	"${SRC_DIR}/wlbl_label.cpp"
	"${SRC_DIR}/wlbl_reasonwise_label.cpp"
	
	"${SRC_DIR}/view_matched_comments.cpp"
	
	# Utils
	"${UTILS_SRC_DIR}/id2str.cpp"
	"${SRC_DIR}/btn_with_id.cpp"
	"${SRC_DIR}/name_dialog.cpp"
	"${SRC_DIR}/sql_name_dialog.cpp"
	
	"${SRC_DIR}/msgbox.cpp"
	
	"${SHARED_SRC_DIR}/init_regexp_from_file.cpp"
	
	# Messages
	"${SRC_DIR}/notfound.cpp"
	
	"${SRC_DIR}/regex_editor/editor.cpp"
	
	# 3rd party
	"${SRC_DIR}/3rdparty/donutbreakdownchart.cpp"
	"${SRC_DIR}/3rdparty/mainslice.cpp"
	"${SRC_DIR}/3rdparty/codeeditor.cpp"
	
	# Tag Colour Picker tabs
	"${SRC_DIR}/categorytab.cpp"
	"${SRC_DIR}/reason_tab.cpp"
	"${SRC_DIR}/io_tab.cpp"
	"${SRC_DIR}/clbtn.cpp"
	"${SRC_DIR}/tagnamelabel.cpp"
	"${SRC_DIR}/add_sub2tag_btn.cpp"
	"${SRC_DIR}/sh_tag_btn.cpp"
	"${SRC_DIR}/unlink_tag_btn.cpp"
	"${SRC_DIR}/rm_tag_btn.cpp"
	
	"${SRC_DIR}/regex_tests.cpp"

	"${SRC_DIR}/cat_doughnut.cpp"
	"${SRC_DIR}/cat_pie.cpp"
	"${SRC_DIR}/tag_pie.cpp"
)
set_property(TARGET rscraper-hub PROPERTY CXX_STANDARD 17)
target_include_directories(rscraper-hub PRIVATE ${COMPSKY_INCLUDE_DIRS})
target_link_libraries(rscraper-hub Qt5::Widgets Qt5::Charts "${MYSQL_CLIENT}")
if(Boost_FOUND)
	target_link_libraries(rscraper-hub ${Boost_REGEX_LIBRARY} egix)
	target_compile_definitions(rscraper-hub PRIVATE USE_BOOST_REGEX)
endif()

include(GNUInstallDirs)
install(
	TARGETS rscraper-hub
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT gui
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(rscraper-hub secur32)
endif()
