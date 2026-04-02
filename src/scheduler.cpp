#include <specs/private/scheduler.hpp>

namespace specs {
    std::optional<std::tuple<System, std::span<AllocatedQuery>, std::span<size_t>>> Scheduler::get_next_system_and_data() {
        uint32_t i = ++system_index;
        Schedule& s = schedules[schedule_index];
        if (s.frames[frame_index].end_index < i) {
            return std::nullopt;
        }

        System sys = s.systems[s.ordered[i]];
        return std::make_optional(std::make_tuple(sys, std::span(s.allocated_queries), std::span(s.query_data)));
    }
}
