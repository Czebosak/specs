#pragma once

#include <vector>
#include <atomic>

#include <specs/private/system.hpp>
#include <specs/private/schedule.hpp>
#include <specs/private/allocated_query.hpp>

namespace specs {
    class Schedule;
    class WorkerPool;

    class Scheduler {
    private:
        std::vector<Schedule> schedules;

        uint32_t schedule_index = 0;
        uint32_t frame_index = 0;
        std::atomic<uint32_t> system_index = 0;

        bool executing = false;

        friend Schedule;
        friend WorkerPool;

        // Returns false if finished all
        bool advance();

        inline bool is_executing() {
            return executing;
        }

        std::optional<std::tuple<System, std::span<AllocatedQuery>, std::span<size_t>>> get_next_system_and_data();
    };
}
