#pragma once

#include <vector>
#include <atomic>

#include <specs/private/system.hpp>
#include <specs/private/schedule.hpp>

namespace specs {
    class Schedule;

    class Scheduler {
    private:
        std::vector<Schedule> schedules;

        std::atomic<uint32_t> schedule_index;
        std::atomic<uint32_t> system_index;

        friend Schedule;
    public:
        System get_next_system();
    };
}
