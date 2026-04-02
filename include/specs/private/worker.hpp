#pragma once

#include <condition_variable>
#include <thread>

#include <specs/private/system.hpp>

namespace specs {
    class Scheduler;

    class Worker {
    private:
        std::jthread thread;

        std::condition_variable condition_variable;

        const Scheduler* scheduler;

        void run();

        void start(const Scheduler* scheduler);
    public:
        Worker();
    };
}
