import specs.schedule;
import specs.component;
import specs.query;
import specs.world;
import specs.component_storage;

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

int main() {
    specs::World world;

    world.get_schedule().register_system([](Query<Velocity&> q) {
        auto [v] = q.single();
        std::println("{}", v.value[0]);
        v.value[0] += 0.01f;
    });

    while (true)
        world.run();

    return -1;
}
