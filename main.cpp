#include "bench.hpp"

auto main(int argc, char** argv) -> int
{
    auto pl {tcob::platform::HeadlessInit()};

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}
