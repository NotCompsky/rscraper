cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR) # CONTRIBUTIONS WELCOME: Tests of previous/future versions which work or not

project(RScraper C CXX)

set(COMPSKY_MAJOR_VERSION 0)
set(COMPSKY_MINOR_VERSION 1)
set(COMPSKY_PATCH_VERSION 0)
set(COMPSKY_VERSION ${COMPSKY_MAJOR_VERSION}.${COMPSKY_MINOR_VERSION}.${COMPSKY_PATCH_VERSION})

# BEGIN src: https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
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
add_subdirectory(tags)
add_subdirectory(rtagger)
add_subdirectory(utils)

# Add all targets to the build-tree export set
export(TARGETS rscraper-tagger FILE "${PROJECT_BINARY_DIR}/RScraperTargets.cmake")

# Registers the build-tree with a global CMake-registry
export(PACKAGE RScraper)


# Create the RScraperConfig.cmake and RScraperConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")

# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(RScraperConfig.cmake.in "${PROJECT_BINARY_DIR}/RScraperConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${COMPSKY_CMAKE_DIR}/${REL_INCLUDE_DIR}")
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


if(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/man/man1")
    # Manpage creation strongly based on example by Michael Wild in CMake mailing list in September 2010
    # https://cmake.org/pipermail/cmake/2010-September/039781.html
    set(MAN_PAGES_BASENAMES "rscraper.1" "rscraper-tags.1" "rscraper-str2id.1" "rscraper-id2str.1" "rscraped-tagged-subs.1" "rscraped-reason.1" "rscrape-mods.1" "rscrape-cmnts.1" "rscraper-init.1")
    set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(MAN_FILES)
    find_program(PANDOC NAMES pandoc)
    foreach(basename ${MAN_PAGES_BASENAMES})
        set(tgt "${basename}")
        add_custom_command(
            COMMAND "${PANDOC}" -s -t man "${DOC_DIR}/${basename}.md" -o "${CMAKE_CURRENT_BINARY_DIR}/${basename}"
            COMMENT "Generating manpage from markdown doc: ${basename}"
            DEPENDS "${DOC_DIR}/${basename}.md"
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${basename}"
        )
        list(APPEND MAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${basename}")
    endforeach()

    add_custom_target(man ALL DEPENDS ${MAN_FILES})
    INSTALL(
        FILES ${MAN_FILES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1"
    )
endif()
