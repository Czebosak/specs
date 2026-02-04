import specs.schedule;
import specs.component;
import specs.query;
import specs.world;
import specs.storage;

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

int main() {
    specs::World world;

    /* world.get_schedule().register_system([](Query<Velocity&> q) {
        auto [v] = q.single();
        v.value[0] += 0.01f;
    }); */

    world.get_schedule().register_system([](Query<Position&, Velocity&> q) {
        auto [p, v] = q.single();
        p.value[0] -= 0.01f;
        v.value[0] += 0.01f;
        std::println("{}, {}", p.value[0], v.value[1]);
    });

    while (true)
        world.run();

    return -1;
}
