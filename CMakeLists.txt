Include(FetchContent)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG v1.9.0
)

if(MSVC)
  set(HAVE_POSIX_REGEX OFF)
endif()

set(BENCHMARK_USE_BUNDLED_GTEST OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)

FetchContent_MakeAvailable(benchmark)

add_executable(tcob_benchmarks)

set(BENCH_SRCFILES
  main.cpp
  DataObjectBenchmarks.cpp
  ImageBenchmarks.cpp
  QuadTreeBenchmarks.cpp
  RandomBenchmarks.cpp
  TransformBenchmarks.cpp
)

if(TCOB_ENABLE_ADDON_SCRIPTING_LUA)
  list(APPEND BENCH_SRCFILES
    LuaScriptBenchmarks.cpp
  )
endif()

target_compile_definitions(tcob_benchmarks PRIVATE BENCHMARK_STATIC_DEFINE)

target_sources(tcob_benchmarks PRIVATE ${BENCH_SRCFILES})

if(NOT TCOB_BUILD_SHARED)
  target_link_libraries(tcob_benchmarks PRIVATE benchmark::benchmark PRIVATE tcob_static)
else()
  target_link_libraries(tcob_benchmarks PRIVATE benchmark::benchmark PRIVATE tcob_shared)
endif()

target_include_directories(tcob_benchmarks PRIVATE . PRIVATE ../../tcob/include)

set_target_properties(tcob_benchmarks PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
)

file(COPY testfiles DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")