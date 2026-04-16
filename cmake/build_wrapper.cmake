function(add_fat_component component objects compile_flags sources isPIC)
    set(BUILD_WRAPPER "${PROJECT_SOURCE_DIR}/cmake/build_wrapper.sh")
    add_library(${objects} OBJECT ${sources})
    set_target_properties(${objects} PROPERTIES
        COMPILE_FLAGS "${compile_flags}"
	POSITION_INDEPENDENT_CODE ${isPIC}
        RULE_LAUNCH_COMPILE "${BUILD_WRAPPER} ${component} ${CMAKE_MODULE_PATH}/keep.syms.in"
        )
endfunction()
