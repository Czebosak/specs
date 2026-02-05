module;

#include <vector>
#include <atomic>

export module specs.scheduler;

import specs.system;
import specs.schedule;

namespace specs {
    export class Scheduler {
    private:
        std::vector<Schedule> schedules;

        std::atomic<uint32_t> schedule_index;
        std::atomic<uint32_t> system_index;
    public:
        System get_next_system() {
            if (schedules[schedule_index])
        }
    };
}
