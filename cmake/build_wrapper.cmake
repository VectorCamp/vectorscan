function(add_fat_component component objects compile_flags sources isPIC)
    add_library(${objects} OBJECT ${sources})
    if (WINDOWS)
        set(BUILD_WRAPPER "${PROJECT_SOURCE_DIR}/cmake/build_wrapper_mingw.sh")
        set_target_properties(${objects} PROPERTIES
            COMPILE_FLAGS "${compile_flags}"
            POSITION_INDEPENDENT_CODE ${isPIC}
	    RULE_LAUNCH_COMPILE "C:\\MSYS64\\usr\\bin\\bash.exe -l ${BUILD_WRAPPER} ${component} ${CMAKE_MODULE_PATH}/keep.syms.in"
            )
    else()
        set(BUILD_WRAPPER "${PROJECT_SOURCE_DIR}/cmake/build_wrapper.sh")
        set_target_properties(${objects} PROPERTIES
            COMPILE_FLAGS "${compile_flags}"
            POSITION_INDEPENDENT_CODE ${isPIC}
            RULE_LAUNCH_COMPILE "${BUILD_WRAPPER} ${component} ${CMAKE_MODULE_PATH}/keep.syms.in"
            )
    endif()
endfunction()
