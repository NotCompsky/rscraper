if(NOT IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/man/man1")
    return()
endif()

# Manpage creation strongly based on example by Michael Wild in CMake mailing list in September 2010
# https://cmake.org/pipermail/cmake/2010-September/039781.html
set(MAN_PAGES_BASENAMES "rscraper.1" "rscraper-tags.1" "rscraper-str2id.1" "rscraper-id2str.1" "rscraped-tagged-subs.1" "rscraped-reason.1" "rscrape-mods.1" "rscrape-cmnts.1" "rscraper-init.1" "rscraper-export.1")
set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
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
