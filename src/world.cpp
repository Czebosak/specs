#include <specs/private/worker.hpp>

#include <specs/world.hpp>

namespace specs {
    World::World(unsigned int worker_count) : worker_pool(worker_count, scheduler, storage) {}

    World::~World() = default;

    void World::run() {
        scheduler.prepare();
        worker_pool.start();
    }

    void World::run_blocking() {
        run();
        worker_pool.wait();
    }
}
