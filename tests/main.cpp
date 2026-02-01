import specs.schedule;
import specs.component;
import specs.query;
import specs.world;

#include <print>

template<specs::QueriedComponentType... QueriedComponents>
using Query = specs::Query<QueriedComponents...>;

struct Position {
    float x, y, z;
};

struct Velocity {
    float x, y, z;
};

struct Player {
    float x, y, z;
};

int main() {
    specs::World world;
    /* specs::EntityHandle e = world.create_entity();

    std::cout << e.id << e.generation << std::endl;

    e = specs::EntityHandle { world.create_entity() };

    std::cout << e.id << e.generation << std::endl; */

    //schedule.register_system([](Position& pos, const Velocity& veltime) {});

    world.get_schedule().register_system([](Query<const Position&>) {
        std::println("skibdi tetst");
    });

    world.run();

    std::println("WE ARE HEERE");

    return -1;
}
