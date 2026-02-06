#include <specs/world.hpp>

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

int main() {
    specs::World world;

    world.get_schedule().register_system([](Query<Velocity&> q) {
        auto [v] = q.single();
        v.value[0] += 0.01f;
    });

    world.get_schedule().register_system([](Query<Position&> q) {
        auto [p] = q.single();
        p.value[0] -= 0.01f;
    });

    world.get_schedule().register_system([](Query<const Position&, const Velocity&> q) {
        auto [p, v] = q.single();
        std::println("{}, {}", p.value[0], v.value[0]);
    });

    while (true)
        world.run();

    return -1;
}
