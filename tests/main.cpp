#include <csignal>
#include <specs/world.hpp>

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

struct Main {};

std::chrono::steady_clock::time_point startTime;
int i = 0;

void signalHandler(int signum) {
    auto endTime = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

    std::println("Elapsed time: {} seconds", delta);
    std::println("Iterations: {}", i);
    std::println("Iterations per second: {}", static_cast<double>(i) / static_cast<double>(delta));

    exit(signum);
}

int main() {
    std::signal(SIGINT, signalHandler);

    specs::World world;

    world.add_systems(Main{}, [](Query<Velocity&> q) {
        auto [v] = q.single();
        v.value[0] += 0.01f;
        //std::println("no");
    });

    world.add_systems(Main{}, [](Query<const Position&, const Velocity&> q) {
        auto [p, v] = q.single();
        //std::println("{}, {}", p.value[0], v.value[0]);
        //std::println("epic");
    });

    world.add_systems(Main{}, [](Query<Position&> q) {
        auto [p] = q.single();
        p.value[0] -= 0.01f;
        //std::println("yes");
    });

    startTime = std::chrono::steady_clock::now();


    while (true) {
        world.run_blocking();
        i++;
    }

    return 0;
}
