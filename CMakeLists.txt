cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

if( EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Refusing to run in-source build.")
endif()

project(RScraper C CXX) # WARNING: Sets some important variables about the plarform. Don't call find_package before setting a project name.




if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # e.g. cmake .. -DCMAKE_BUILD_TYPE=Debug
    set(MY_DEFINITIONS "DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined -Wall -Wextra -Wshadow -Wstrict-aliasing -Wfloat-equal -Wpointer-arith -Wwrite-strings -Wformat=2 -Wstrict-overflow -Wunreachable-code") # -Wredundant-decls -Wmissing-declarations
    endif()
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(MY_DEFINITIONS "QT_NO_DEBUG" "QT_NO_DEBUG_OUTPUT")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if (NOT WHICH_MYSQL_CLIENT)
    find_library(FOUND_MYSQL_CLIENT mariadbclient)
    if (FOUND_MYSQL_CLIENT)
        set(WHICH_MYSQL_CLIENT mariadbclient)
    endif()
endif()
if (NOT WHICH_MYSQL_CLIENT)
    find_library(FOUND_MYSQL_CLIENT mysqlclient)
    if (FOUND_MYSQL_CLIENT)
        set(WHICH_MYSQL_CLIENT mysqlclient)
    endif()
endif()
if (WHICH_MYSQL_CLIENT)
    message(STATUS "WHICH_MYSQL_CLIENT: ${WHICH_MYSQL_CLIENT}")
else()
    message(FATAL_ERROR "Please specify the libmysqlclient equivalent with -DWHICH_MYSQL_CLIENT=...\nFor instance, for Ubuntu/Debian, you would specify -DWHICH_MYSQL_CLIENT=mariadbclient; for others, perhaps, -DWHICH_MYSQL_CLIENT=mysqlclient\nFor Ubuntu/Debian, you can discover this with the command:  apt list --installed | egrep 'lib(mysql|mariadb)client' | grep -v -- -dev | sed 's/^lib//g' | sed -r 's~[0-9]*/.*~~g'")
endif()



if(WHY_THIS_NECESSARY)
    set(COMPSKY_INCLUDE_DIRS "${WHY_THIS_NECESSARY}/../include")
    set(COMPSKY_BIN_DIRS "${WHY_THIS_NECESSARY}/../bin")
    set(COMPSKY_LIB_DIRS "${WHY_THIS_NECESSARY}")
else()
    find_package(Compsky COMPONENTS asciify mysql REQUIRED)
    # Gives that `*** No rule to make target 'compsky_asciify-NOTFOUND'` error if defined only in individual subdirectories' CMakeLists.
endif()


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS "TRUE") # For Windows Visual Studio # Doesn't seem to do anything though!

set(RSCRAPER_MAJOR_VERSION 0)
set(RSCRAPER_MINOR_VERSION 1)
set(RSCRAPER_PATCH_VERSION 0)
set(RSCRAPER_VERSION ${RSCRAPER_MAJOR_VERSION}.${RSCRAPER_MINOR_VERSION}.${RSCRAPER_PATCH_VERSION})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lib")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/shared-src")

include_directories("${COMPSKY_INCLUDE_DIRS}")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    link_directories("${COMPSKY_BIN_DIRS}")
else()
    link_directories("${COMPSKY_LIB_DIRS}")
endif()


if(BUILD_PACKAGES)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    SET(CPACK_RPM_COMPONENT_INSTALL     ON)
    SET(CPACK_DEB_COMPONENT_INSTALL     ON)
    set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)


    if("${BUILD_PACKAGES}" STREQUAL "1")
        set(CPACK_COMPONENTS_ALL utils init io man scraper tagger gui)
        message(STATUS "Note: You can specify the packages to build with -DBUILD_PACKAGES=<package_name>")
    else()
        set(CPACK_COMPONENTS_ALL ${BUILD_PACKAGES})
    endif()


    set(CPACK_COMPONENT_UTILS_DESCRIPTION
        "Slightly useful utilities for rscraper")
    set(CPACK_COMPONENT_INIT_DESCRIPTION
        "Utility for initialising the database necessary for all rscraper packages")
    set(CPACK_COMPONENT_IO_DESCRIPTION
        "Utility for importing to and exporting from the RScraper database")
    set(CPACK_COMPONENT_MAN_DESCRIPTION
        "Man pages for all binaries")
    set(CPACK_COMPONENT_SCRAPER_DESCRIPTION
        "Tools for scraping Reddit")
    set(CPACK_COMPONENT_TAGGER_DESCRIPTION
        "Server for RTagger")
    set(CPACK_COMPONENT_GUI_DESCRIPTION
        "Qt5-based GUI for managing the scraper and tagger")
    
    set(Boost_USE_STATIC_LIBS ON) # Different repos have different libboost-regex versions that different repositories include, just easier to avoid this dependency for packages.
    set(CPACK_DEBIAN_UTILS_PACKAGE_DEPENDS "libcompsky")
    set(CPACK_DEBIAN_INIT_PACKAGE_DEPENDS "libcompsky")
    set(CPACK_DEBIAN_IO_PACKAGE_DEPENDS "libcompsky")
    set(CPACK_DEBIAN_MAN_PACKAGE_DEPENDS "")
    set(CPACK_DEBIAN_SCRAPER_PACKAGE_DEPENDS "libcompsky,  libcurl4 | libcurl3")
    set(CPACK_DEBIAN_TAGGER_PACKAGE_DEPENDS "libcompsky")
    set(CPACK_DEBIAN_GUI_PACKAGE_DEPENDS "libcompsky, libqt5widgets5, libqt5charts5")
endif()

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("#include <filesystem>" FS_NOT_EXPERIMENTAL)


# BEGIN src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/RScraper)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
include_directories("${PROJECT_SOURCE_DIR}")

# Add sub-directories
add_subdirectory(scraper)
add_subdirectory(hub)
add_subdirectory(tagger)
add_subdirectory(init)
add_subdirectory(utils)
add_subdirectory(io)
add_subdirectory(man)
if(BUILD_PACKAGES)
    add_subdirectory(packaging)
endif()

# Registers the build-tree with a global CMake-registry
export(PACKAGE RScraper)


# Create the RScraperConfig.cmake and RScraperConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")

# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(RScraperConfig.cmake.in "${PROJECT_BINARY_DIR}/RScraperConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${RSCRAPER_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(RScraperConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/RScraperConfig.cmake" @ONLY)
# ... for both
configure_file(RScraperConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/RScraperConfigVersion.cmake" @ONLY)

# Install the RScraperConfig.cmake and RScraperConfigVersion.cmake
install(
    FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/RScraperConfig.cmake"
    "${PROJECT_BINARY_DIR}/RScraperConfigVersion.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev
)

# Install the export set for use with the install-tree
install(EXPORT RScraperTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# END src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
