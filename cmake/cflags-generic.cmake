# set compiler flags - more are tested and added later
set(EXTRA_C_FLAGS "${OPT_C_FLAG} -std=c17 -Wall -Wextra ")
set(EXTRA_CXX_FLAGS "${OPT_CXX_FLAG} -std=c++17 -Wall -Wextra ")
if (NOT CMAKE_COMPILER_IS_CLANG)
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -fno-new-ttp-matching")
endif()

# Always use -Werror *also during release builds
set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Werror")
set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wall -Werror")

if (DISABLE_ASSERTS)
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -DNDEBUG")
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -DNDEBUG")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    # spurious warnings?
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-array-bounds ") #-Wno-maybe-uninitialized")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-maybe-uninitialized -Wno-uninitialized")
endif()

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)
CHECK_FUNCTION_EXISTS(_aligned_malloc HAVE__ALIGNED_MALLOC)

if(FREEBSD OR NETBSD)
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -gdwarf-4")
endif()

if(NETBSD)
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -DHAVE_BUILTIN_POPCOUNT")
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -DHAVE_BUILTIN_POPCOUNT")
endif()

if(MACOSX)
    # Boost headers cause such complains on MacOS
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-deprecated-declarations -Wno-unused-parameter")
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-parameter")
endif()

# these end up in the config file
CHECK_C_COMPILER_FLAG(-fvisibility=hidden HAS_C_HIDDEN)
CHECK_CXX_COMPILER_FLAG(-fvisibility=hidden HAS_CXX_HIDDEN)

# are we using libc++
CHECK_CXX_SYMBOL_EXISTS(_LIBCPP_VERSION ciso646 HAVE_LIBCPP)

if (RELEASE_BUILD)
    if (HAS_C_HIDDEN)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fvisibility=hidden")
    endif()
    if (HAS_CXX_HIDDEN)
        set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

# testing a builtin takes a little more work
CHECK_C_SOURCE_COMPILES("void *aa_test(void *x) { return __builtin_assume_aligned(x, 16);}\nint main(void) { return 0; }" HAVE_CC_BUILTIN_ASSUME_ALIGNED)
CHECK_CXX_SOURCE_COMPILES("void *aa_test(void *x) { return __builtin_assume_aligned(x, 16);}\nint main(void) { return 0; }" HAVE_CXX_BUILTIN_ASSUME_ALIGNED)
# Clang does not use __builtin_constant_p() the same way as gcc
if (NOT CMAKE_COMPILER_IS_CLANG)
   CHECK_C_SOURCE_COMPILES("int main(void) { __builtin_constant_p(0); }" HAVE__BUILTIN_CONSTANT_P)
endif()

# clang-14 complains about unused-but-set variable.
CHECK_CXX_COMPILER_FLAG("-Wunused-but-set-variable" CXX_UNUSED_BUT_SET_VAR)
if (CXX_UNUSED_BUT_SET_VAR)
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-unused-but-set-variable")
endif()

CHECK_CXX_COMPILER_FLAG("-Wignored-attributes" CXX_IGNORED_ATTR)
if(CMAKE_COMPILER_IS_GNUCC)
    if (CXX_IGNORED_ATTR)
        set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-ignored-attributes")
    endif()
endif()

CHECK_CXX_COMPILER_FLAG("-Wignored-attributes" CXX_NON_NULL)
if(CMAKE_COMPILER_IS_GNUCC)
    if (CXX_NON_NULL)
        set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-nonnull")
    endif()
endif()

# note this for later, g++ doesn't have this flag but clang does
CHECK_CXX_COMPILER_FLAG("-Wweak-vtables" CXX_WEAK_VTABLES)

CHECK_CXX_COMPILER_FLAG("-Wmissing-declarations" CXX_MISSING_DECLARATIONS)

CHECK_CXX_COMPILER_FLAG("-Wunused-local-typedefs" CXX_UNUSED_LOCAL_TYPEDEFS)

CHECK_CXX_COMPILER_FLAG("-Wunused-variable" CXX_WUNUSED_VARIABLE)

# gcc complains about this
if(CMAKE_COMPILER_IS_GNUCC)
    CHECK_C_COMPILER_FLAG("-Wstringop-overflow" CC_STRINGOP_OVERFLOW)
    CHECK_CXX_COMPILER_FLAG("-Wstringop-overflow" CXX_STRINGOP_OVERFLOW)
    if(CC_STRINGOP_OVERFLOW OR CXX_STRINGOP_OVERFLOW)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-stringop-overflow -Wno-stringop-overread")
        set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-stringop-overflow -Wno-stringop-overread")
    endif()
endif()
