#pragma once

#include <barrier>
#include <functional>
#include <print>
#include <vector>
#include <thread>
#include <condition_variable>

namespace specs {
    class Scheduler;
    class Storage;

    class WorkerPool {
    private:
        std::vector<std::jthread> workers;

        std::mutex queue_mutex;
        std::condition_variable condition;

        Scheduler& scheduler;
        Storage& storage;

        std::barrier<std::move_only_function<void()>> barrier;

        bool stop = false;
    public:
        WorkerPool(size_t threads, Scheduler& scheduler, Storage& storage);

        WorkerPool(const WorkerPool&) = delete;
        WorkerPool& operator=(const WorkerPool&) = delete;
        WorkerPool(WorkerPool&&) = delete;
        WorkerPool& operator=(WorkerPool&&) = delete;

        void start();
    };
}
