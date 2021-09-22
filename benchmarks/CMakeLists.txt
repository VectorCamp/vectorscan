add_executable(benchmarks benchmarks.cpp)
set_source_files_properties(benchmarks.cpp PROPERTIES COMPILE_FLAGS
    "-Wall -Wno-unused-variable")
target_link_libraries(benchmarks hs)
