
CHECK_INCLUDE_FILE_CXX(altivec.h HAVE_C_PPC64EL_ALTIVEC_H)

if (HAVE_C_PPC64EL_ALTIVEC_H)
    set (INTRIN_INC_H "altivec.h")
else()
    message (FATAL_ERROR "No intrinsics header found for VSX")
endif ()

CHECK_C_SOURCE_COMPILES("#include <${INTRIN_INC_H}>
int main() {
    vector int a = vec_splat_s32(1);
    (void)a;
}" HAVE_VSX)

if (NOT HAVE_VSX)
    message(FATAL_ERROR "VSX support required for Power support")
endif ()

# fix unit-internal seg fault for freebsd and gcc13
if (FREEBSD AND CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "13")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif ()
endif ()
