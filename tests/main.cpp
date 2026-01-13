import specs.world;
import specs.entity;

#include <iostream>

int main() {
    specs::World world;
    specs::EntityHandle e = world.create_entity();

    std::cout << e.id << e.generation << std::endl;

    e = specs::EntityHandle { world.create_entity() };

    std::cout << e.id << e.generation << std::endl;
    return -1;
}