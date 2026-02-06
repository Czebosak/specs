#include <specs/private/worker.hpp>

#include <specs/world.hpp>

specs::World::World(unsigned int worker_count) : workers(worker_count) {}

specs::World::~World() = default;
