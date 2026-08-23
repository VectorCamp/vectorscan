find_program(DOXYGEN doxygen)

if (DOXYGEN STREQUAL DOXYGEN-NOTFOUND)
    message(STATUS "Doxygen not found, unable to generate API reference")
else()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/hyperscan.doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hyperscan.doxyfile" @ONLY)

add_custom_target(dev-reference-doxygen
    ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/hyperscan.doxyfile
    COMMENT "Building doxygen XML for API reference")
endif()

find_program(SPHINX_BUILD sphinx-build)

if (SPHINX_BUILD STREQUAL SPHINX_BUILD-NOTFOUND)
    message(STATUS "Sphinx not found, unable to generate developer reference")
else()
set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
set(SPHINX_MAN_DIR "${CMAKE_CURRENT_BINARY_DIR}/man")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${CMAKE_CURRENT_BINARY_DIR}/conf.py" @ONLY)

add_custom_target(dev-reference
    ${SPHINX_BUILD}
        -b html
        -c "${CMAKE_CURRENT_BINARY_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_DIR}"
    DEPENDS dev-reference-doxygen
    COMMENT "Building HTML dev reference with Sphinx")

add_custom_target(dev-reference-man ALL
    ${SPHINX_BUILD}
        -b man
        -c "${CMAKE_CURRENT_BINARY_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_MAN_DIR}"
    DEPENDS dev-reference-doxygen
    COMMENT "Building man page reference with Sphinx")

install(FILES ${CMAKE_BINARY_DIR}/doc/dev-reference/man/vectorscan.7
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man7/")
endif()
