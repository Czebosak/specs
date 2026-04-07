#include <specs/world.hpp>

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

struct Main {};

int main() {
    specs::World world;

    world.add_systems(Main{}, [](Query<Velocity&> q) {
        auto [v] = q.single();
        v.value[0] += 0.01f;
        std::println("no");
    });

    world.add_systems(Main{}, [](Query<const Position&, const Velocity&> q) {
        auto [p, v] = q.single();
        std::println("{}, {}", p.value[0], v.value[0]);
        std::println("epic");
    });

    world.add_systems(Main{}, [](Query<Position&> q) {
        auto [p] = q.single();
        p.value[0] -= 0.01f;
        std::println("yes");
    });

    for (int i = 1; i <= 10; i++) {
        std::println("{}", i);
        world.run_blocking();
    }

    return 0;
}
