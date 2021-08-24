add_executable(benchmarks benchmarks.cpp shufti.cpp truffle.cpp noodle.cpp)
set_source_files_properties(shufti.cpp PROPERTIES COMPILE_FLAGS
    "-Wall -Wno-unused-variable")
target_link_libraries(benchmarks hs)