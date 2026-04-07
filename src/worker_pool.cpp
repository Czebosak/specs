#include <print>
#include <specs/private/worker_pool.hpp>

#include <specs/private/scheduler.hpp>

#include <access_private.hpp>

template struct access_private::access<&std::barrier<std::move_only_function<void()>>::_M_b>;
        //assert(reinterpret_cast<size_t*>(&access_private::accessor<"_M_b">(barrier))[1] != 0);

namespace specs {

    WorkerPool::WorkerPool(size_t threads, Scheduler& scheduler, Storage& storage)
    : scheduler(scheduler),
      storage(storage),
      barrier(threads, [this] {
        bool advance = this->scheduler.advance();
        if (advance) condition.notify_all();
    }) {
        assert(threads > 0);
        workers.reserve(threads);
        for (size_t i = 0; i < threads; i++) {
            workers.emplace_back([this] {
                while (true) {
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || this->scheduler.is_executing(); });
                        if (stop) { return; }
                    }

                    while (true) {
                        auto opt = this->scheduler.get_next_system_and_data();
                        if (opt) {
                            auto [system, allocated_queries, query_data] = opt.value();
                            system.func(this->storage, std::span<AllocatedQuery>{&allocated_queries[system.query_list_index], system.query_count}, query_data);
                        } else {
                            barrier.arrive_and_wait();
                            break;
                        }
                    }
                }
            });
        }
    }

    void WorkerPool::start() {
        condition.notify_all();
    }
}
