if (WINDOWS)
    set(LIBC "libucrtbase.a")
    set(LIBCSYMS ${LIBC}.syms)
    add_custom_target(${LIBCSYMS} COMMAND ${CMAKE_COMMAND} -E env "PATH=C:\\msys64\\ucrt64\\bin;C:\\msys64\\usr\\bin;%PATH%"
	              bash ${CMAKE_MODULE_PATH}/libcsyms.sh ${CMAKE_C_COMPILER} ${LIBC} ${LIBCSYMS} -j
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    set(LIBC "libc.so.6")
    set(LIBCSYMS ${LIBC}.syms)
    add_custom_target(${LIBCSYMS} COMMAND bash ${CMAKE_MODULE_PATH}/libcsyms.sh ${CMAKE_C_COMPILER} ${LIBC} ${LIBCSYMS} -D
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

function(add_fat_component component objects compile_flags sources isPIC)
    if (WINDOWS)
	set(BUILD_WRAPPER "${PROJECT_SOURCE_DIR}/cmake/build_wrapper_mingw.sh")
	if (MSYS OR MINGW)
            set(BUILD_WRAPPER "bash ${PROJECT_SOURCE_DIR}/cmake/build_wrapper_mingw.sh")
	endif()
    else()
        set(BUILD_WRAPPER "${PROJECT_SOURCE_DIR}/cmake/build_wrapper.sh")
    endif()
    set(KEEPSYMS "${CMAKE_MODULE_PATH}/keep.syms.in")
    add_library(${objects} OBJECT ${sources})
    add_dependencies(${objects} ${LIBCSYMS})
    set_target_properties(${objects} PROPERTIES
        COMPILE_FLAGS "${compile_flags}"
	POSITION_INDEPENDENT_CODE ${isPIC}
	RULE_LAUNCH_COMPILE "${BUILD_WRAPPER} ${component} ${KEEPSYMS} ${LIBCSYMS}"
        )
endfunction()
