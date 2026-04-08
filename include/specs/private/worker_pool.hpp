#pragma once

#include <barrier>
#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>

namespace specs {
    class Scheduler;
    class Storage;
    class World;

    class WorkerPool {
    private:
        std::vector<std::jthread> workers;

        std::mutex queue_mutex;
        std::condition_variable condition;

        std::mutex finished_mutex;
        std::condition_variable finished;

        Scheduler& scheduler;
        Storage& storage;

        std::barrier<std::move_only_function<void()>> barrier;

        bool next_frame_ready = false;
        bool stop = false;
        friend World;
    public:
        WorkerPool(size_t threads, Scheduler& scheduler, Storage& storage);

        ~WorkerPool();

        WorkerPool(const WorkerPool&) = delete;
        WorkerPool& operator=(const WorkerPool&) = delete;
        WorkerPool(WorkerPool&&) = delete;
        WorkerPool& operator=(WorkerPool&&) = delete;

        void start();

        void wait();
    };
}
