#include "bench.hpp"

#include <random>

struct Node {
    rect_f      box;
    std::size_t id {};

    auto get_rect() const -> rect_f { return box; }
    auto operator==(Node const& other) const -> bool = default;
};

auto generateRandomNodes(std::size_t n) -> std::vector<Node>
{
    auto generator          = std::default_random_engine();
    auto originDistribution = std::uniform_real_distribution(0.0f, 1.0f);
    auto sizeDistribution   = std::uniform_real_distribution(0.0f, 0.01f);
    auto nodes              = std::vector<Node>(n);
    for (auto i = std::size_t(0); i < n; ++i) {
        nodes[i].box.Position.X  = originDistribution(generator);
        nodes[i].box.Position.Y  = originDistribution(generator);
        nodes[i].box.Size.Width  = std::min(1.0f - nodes[i].box.left(), sizeDistribution(generator));
        nodes[i].box.Size.Height = std::min(1.0f - nodes[i].box.top(), sizeDistribution(generator));
        nodes[i].id              = i;
    }
    return nodes;
}

auto query(rect_f const& box, std::vector<Node>& nodes) -> std::vector<Node>
{
    auto intersections = std::vector<Node>();
    for (auto& n : nodes) {
        if (box.intersects(n.box)) { intersections.push_back(n); }
    }
    return intersections;
}

auto findAllIntersections(std::vector<Node>& nodes) -> std::vector<std::pair<Node, Node>>
{
    auto intersections = std::vector<std::pair<Node, Node>>();
    for (auto i = std::size_t(0); i < nodes.size(); ++i) {
        for (auto j = std::size_t(0); j < i; ++j) {
            if (nodes[i].box.intersects(nodes[j].box)) { intersections.emplace_back(nodes[i], nodes[j]); }
        }
    }
    return intersections;
}

void QUADTREE_Build(benchmark::State& state)
{
    auto box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto intersections = std::vector<std::vector<Node>>(nodes.size());
        auto quadtree      = tcob::gfx::quadtree<Node>(box);
        for (auto& node : nodes) { quadtree.add(node); }
    }
}

void QUADTREE_Query4(benchmark::State& state)
{
    auto box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto intersections = std::vector<std::vector<Node>>(nodes.size());
        auto quadtree      = tcob::gfx::quadtree<Node, 4>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        for (auto const& node : nodes) { intersections[node.id] = quadtree.query(node.box); }
    }
}

void QUADTREE_Query16(benchmark::State& state)
{
    auto box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto intersections = std::vector<std::vector<Node>>(nodes.size());
        auto quadtree      = tcob::gfx::quadtree<Node, 16>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        for (auto const& node : nodes) { intersections[node.id] = quadtree.query(node.box); }
    }
}

void QUADTREE_AddRemove(benchmark::State& state)
{
    auto box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto quadtree = tcob::gfx::quadtree<Node>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        for (auto& node : nodes) { quadtree.remove(node); }
        for (auto& node : nodes) { quadtree.add(node); }
    }
}

void QUADTREE_ReplacePadded(benchmark::State& state)
{
    auto              box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto              nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    std::vector<Node> replnodes;
    replnodes.reserve(nodes.size());
    for (auto& node : nodes) {
        replnodes.push_back({node.box.as_padded_by({0.001f, 0.001})});
    }

    for (auto _ : state) {
        auto quadtree = tcob::gfx::quadtree<Node>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        for (usize i {0}; i < nodes.size(); ++i) {
            quadtree.replace(nodes[i], replnodes[i]);
        }
    }
}

void QUADTREE_ReplaceRandom(benchmark::State& state)
{
    auto box       = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes     = generateRandomNodes(static_cast<std::size_t>(state.range()));
    auto replnodes = generateRandomNodes(static_cast<std::size_t>(state.range()));

    for (auto _ : state) {
        auto quadtree = tcob::gfx::quadtree<Node>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        for (usize i {0}; i < nodes.size(); ++i) {
            quadtree.replace(nodes[i], replnodes[i]);
        }
    }
}

void QUADTREE_FindAllIntersections(benchmark::State& state)
{
    auto box   = rect_f(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto quadtree = tcob::gfx::quadtree<Node>(box);
        for (auto& node : nodes) { quadtree.add(node); }
        auto intersections = quadtree.find_all_intersections();
    }
}

void QUADTREE_BruteForceQuery(benchmark::State& state)
{
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto intersections = std::vector<std::vector<Node>>(nodes.size());
        for (auto const& node : nodes) { intersections[node.id] = query(node.box, nodes); }
    }
}

void QUADTREE_BruteForceFindAllIntersections(benchmark::State& state)
{
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state) {
        auto intersections = findAllIntersections(nodes);
    }
}

BENCHMARK(QUADTREE_Build)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_Query4)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_Query16)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_AddRemove)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_ReplacePadded)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_ReplaceRandom)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_FindAllIntersections)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_BruteForceQuery)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
BENCHMARK(QUADTREE_BruteForceFindAllIntersections)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMillisecond);
