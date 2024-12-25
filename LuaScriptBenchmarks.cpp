#include "bench.hpp"
#include <string>

using namespace tcob::scripting::lua;

constexpr i32 NUM_ITERATIONS {1000000};

////////////////////////////////////////////////////////////

void static LUA_NewScript(benchmark::State& state)
{
    for (auto _ : state) {
        script s {};
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_NewScript)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_RunScript(benchmark::State& state)
{
    script s;
    for (auto _ : state) {
        auto x = s.run("function foo() return 100 end foo()");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_RunScript)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_SetString(benchmark::State& state)
{
    script      s;
    auto&       global = s.global_table();
    std::string str    = "teststring";
    for (auto _ : state) {
        benchmark::DoNotOptimize(global["f"] = str);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_SetString)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_SetStringView(benchmark::State& state)
{
    script                     s;
    auto&                      global = s.global_table();
    constexpr std::string_view str    = "teststring";
    for (auto _ : state) {
        benchmark::DoNotOptimize(global["f"] = str);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_SetStringView)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_GetAndCallFunction(benchmark::State& state)
{
    script                s;
    [[maybe_unused]] auto res = s.run("function foo() return 100 end");
    for (auto _ : state) {
        tcob::scripting::lua::function<int> f {s.global_table()["foo"]};
        auto                                x = f();
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_GetAndCallFunction)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_ProtectedCall(benchmark::State& state)
{
    script                              s;
    [[maybe_unused]] auto               res {s.run("function foo(p,q,r) return p.x + p.y + q.x + q.y + r.x + r.y end")};
    tcob::scripting::lua::function<int> f {s.global_table()["foo"]};
    for (auto _ : state) {
        benchmark::DoNotOptimize(f.protected_call(point_i {10, 30}, point_i {42, 11}, point_i {88, 65}));
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_ProtectedCall)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////

void static LUA_UnprotectedCall(benchmark::State& state)
{
    script                              s;
    [[maybe_unused]] auto               res {s.run("function foo(p,q,r) return p.x + p.y + q.x + q.y + r.x + r.y end")};
    tcob::scripting::lua::function<int> f {s.global_table()["foo"]};
    for (auto _ : state) {
        benchmark::DoNotOptimize(f.unprotected_call(point_i {10, 30}, point_i {42, 11}, point_i {88, 65}));
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_UnprotectedCall)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////
struct p {
    f32 x;
    f32 y;
};
namespace tcob::scripting::lua {
template <>
struct converter<p> {
    auto static IsType(state_view state, i32 idx) -> bool
    {
        if (state.is_table(idx)) {
            table tab {table::Acquire(state, idx)};
            return (tab.is<f32>("x") && tab.is<f32>("y"));
        }
        return false;
    }

    auto static From(state_view state, i32& idx, p& value) -> bool
    {
        if (state.is_table(idx)) {
            table tab {table::Acquire(state, idx++)};
            if (tab.is<f32>("x") && tab.is<f32>("y")) {
                value.x = tab["x"].as<f32>();
                value.y = tab["y"].as<f32>();
                return true;
            }
        }

        return false;
    }

    void static To(state_view state, p const& value)
    {
        table tab {state};

        tab["x"] = value.x;
        tab["y"] = value.y;
    }
};
}

void static LUA_StructConverter(benchmark::State& state)
{
    script                            s;
    [[maybe_unused]] auto             res = s.run("function foo() return { x=100, y=200 } end");
    tcob::scripting::lua::function<p> f {s.global_table()["foo"]};

    for (auto _ : state) {
        p    test {f()};
        auto x = test.x * test.y;
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_StructConverter)->Iterations(NUM_ITERATIONS);

void static LUA_WrapperConverter(benchmark::State& state)
{
    script                               s;
    [[maybe_unused]] auto                res = s.run("function foo(p) p.x=100 p.y=200 end");
    tcob::scripting::lua::function<void> f {s.global_table()["foo"]};
    auto                                 wrap = s.create_wrapper<p>("p");
    wrap->wrap_property<&p::x>("x");
    wrap->wrap_property<&p::y>("y");
    p test {};

    for (auto _ : state) {
        f(&test);
        auto x = test.x * test.y;
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_WrapperConverter)->Iterations(NUM_ITERATIONS);

////////////////////////////////////////////////////////////
