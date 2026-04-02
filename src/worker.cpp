#include <specs/private/worker.hpp>
#include <specs/private/scheduler.hpp>

namespace specs {
    Worker::Worker() : thread(&Worker::run, this) {}

    void Worker::run() {
        // Do work
    }

    void Worker::start(const Scheduler* scheduler) {
        this->scheduler = scheduler;
    }
}
