# Various checks related to Boost

set(BOOST_USE_STATIC_LIBS OFF)
set(BOOST_USE_MULTITHREADED OFF)
set(BOOST_USE_STATIC_RUNTIME OFF)
if (HAVE_LIBCPP)
    # we need a more recent boost for libc++
    set(BOOST_MINVERSION 1.61.0)
else ()
    set(BOOST_MINVERSION 1.57.0)
endif ()
set(BOOST_NO_BOOST_CMAKE ON)

unset(Boost_INCLUDE_DIR CACHE)
# we might have boost in tree, so provide a hint and try again
set(BOOST_INCLUDEDIR "${PROJECT_SOURCE_DIR}/include")
find_package(Boost ${BOOST_MINVERSION} QUIET)
if(NOT Boost_FOUND)
    # otherwise check for Boost installed on the system
    unset(BOOST_INCLUDEDIR)
    find_package(Boost ${BOOST_MINVERSION} QUIET)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost ${BOOST_MINVERSION} or later not found. Either install system packages if available, extract Boost headers to ${CMAKE_SOURCE_DIR}/include, or set the CMake BOOST_ROOT variable.")
    endif()
endif()

message(STATUS "Boost version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

# libc++ 22 changed std::set/std::map lower_bound and upper_bound for
# comparators that are not a strict weak order (llvm/llvm-project#161366).
# Boost.ICL before 1.92 relies on the old behaviour (boostorg/icl#51), so with
# libc++ 22 or later every interval_set operation that spans more than one
# interval loses intervals, and character classes in UTF-8 mode match input
# they exclude: [^A-Za-z] matches "a". Reject that combination unless the ICL
# fix (boostorg/icl#54) is present, either in Boost 1.92 or later or backported,
# or libc++ is told to keep the old lookups, which libc++ 22 and 23 still allow.
# The test only compiles when the combination is broken, so a failure for any
# other reason does not reject the build.
include(CheckCXXSourceCompiles)
set (saved_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
list(APPEND CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIRS})
unset(BOOST_ICL_BROKEN_WITH_LIBCXX CACHE)
CHECK_CXX_SOURCE_COMPILES("
#include <set>
#include <boost/version.hpp>
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION < 220000 || BOOST_VERSION >= 109200
#  error not affected: not libc++ 22 or later, or Boost 1.92 or later
#elif __has_include(<boost/icl/detail/assoc_container_adaptor.hpp>)
#  error not affected: this Boost.ICL has the fix
#elif _LIBCPP_VERSION < 240000 && defined(_LIBCPP_ENABLE_LEGACY_TREE_LOWER_UPPER_BOUND)
#  error not affected: libc++ keeps the old lookups
#endif
int main() { return 0; }
" BOOST_ICL_BROKEN_WITH_LIBCXX)
set (CMAKE_REQUIRED_INCLUDES "${saved_INCLUDES}")

if (BOOST_ICL_BROKEN_WITH_LIBCXX)
    message(FATAL_ERROR "Boost ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} cannot be used with libc++ 22 or later: "
        "its ICL loses intervals under the new std::set lookups, which makes character classes match input they exclude "
        "(https://github.com/VectorCamp/vectorscan/issues/419). Use Boost 1.92 or later, or build against libstdc++. "
        "With libc++ 22 or 23 you can instead add -D_LIBCPP_ENABLE_LEGACY_TREE_LOWER_UPPER_BOUND to CMAKE_CXX_FLAGS.")
endif()

# Boost 1.62 has a bug that we've patched around, check if it is required
if (Boost_VERSION EQUAL 106200)
    set (CMAKE_REQUIRED_INCLUDES ${BOOST_INCLUDEDIR} "${PROJECT_SOURCE_DIR}/include")
    set (BOOST_REV_TEST "
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/concept/assert.hpp>

int main(int,char*[])
{
  using namespace boost;
  // Check const reverse_graph
  {
    typedef adjacency_list< vecS, vecS, bidirectionalS,
      property<vertex_color_t, int>,
      property<edge_weight_t, int>,
      property<graph_name_t, std::string>
    > AdjList;
    typedef reverse_graph<AdjList> Graph;
    BOOST_CONCEPT_ASSERT(( BidirectionalGraphConcept<Graph> ));
  }
  return 0;
}
")

    CHECK_CXX_SOURCE_COMPILES("${BOOST_REV_TEST}" BOOST_REVGRAPH_OK)

    if (NOT BOOST_REVGRAPH_OK)
        message(STATUS "trying patched")
        CHECK_CXX_SOURCE_COMPILES("
#include <boost-patched/graph/reverse_graph.hpp>
${BOOST_REV_TEST}" BOOST_REVGRAPH_PATCH)
    endif()

    if (NOT BOOST_REVGRAPH_OK AND NOT BOOST_REVGRAPH_PATCH)
        message(FATAL_ERROR "Something is wrong with this copy of boost::reverse_graph")
    endif()

    unset (CMAKE_REQUIRED_INCLUDES)
else ()
    unset(BOOST_REVGRAPH_OK CACHE)
    unset(BOOST_REVGRAPH_PATCH CACHE)
endif () # Boost 1.62.0
