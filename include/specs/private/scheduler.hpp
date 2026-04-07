#pragma once

#include <queue>
#include <atomic>
#include <optional>

#include <specs/private/system.hpp>
#include <specs/private/schedule.hpp>
#include <specs/private/allocated_query.hpp>

namespace specs {
    class Schedule;
    class WorkerPool;

    class ScheduleLabelRegistry {
    public:
        template<typename Label>
        static size_t id() {
            static const size_t value = counter++;
            return value;
        }
    private:
        static size_t counter;
    };

    class Scheduler {
    private:
        ankerl::unordered_dense::map<size_t, Schedule> schedules;
        std::priority_queue<Schedule*> schedule_queue;

        uint32_t frame_index;
        std::atomic<uint32_t> system_index;

        bool is_executing = false;

        friend Schedule;
        friend WorkerPool;

        // Returns false if finished all
        bool advance();

        std::optional<std::tuple<System, std::span<AllocatedQuery>, std::span<size_t>>> get_next_system_and_data();
    public:
        Schedule* add_schedule(size_t id);

        std::optional<Schedule*> get_schedule(size_t id);

        void prepare();
    };
}
