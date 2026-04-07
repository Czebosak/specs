#include <specs/private/scheduler.hpp>

namespace specs {
    size_t ScheduleLabelRegistry::counter = 0;

    bool Scheduler::advance() {
        if (schedule_queue.empty()) {
            executing = false;
            return false;
        }

        frame_index++;

        if (frame_index >= schedule_queue.top()->frames.size()) {
            schedule_queue.pop();
            frame_index = 0;
            if (schedule_queue.empty()) {
                executing = false;
            }
        }

        if (!schedule_queue.empty()) {
            system_index = schedule_queue.top()->frames[frame_index - 1].end_index;
        }

        return !schedule_queue.empty();
    }

    std::optional<std::tuple<System, std::span<AllocatedQuery>, std::span<size_t>>> Scheduler::get_next_system_and_data() {
        if (schedule_queue.empty()) {
            return std::nullopt;
        }
        uint32_t i = system_index.fetch_add(1, std::memory_order_relaxed);
        Schedule& s = *schedule_queue.top();
        if (s.frames[frame_index].end_index <= i) {
            //s.frames[frame_index].end_index = system_index - 1;
            //system_index = s.frames[frame_index].end_index - 1;
            return std::nullopt;
        }

        System sys = s.systems[i];
        return std::make_optional(std::make_tuple(sys, std::span(s.allocated_queries), std::span(s.query_data)));
    }

    Schedule* Scheduler::add_schedule(size_t id) {
        auto [it, b] = schedules.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
        schedule_queue.emplace(&it->second);
        return &it->second;
    }

    std::optional<Schedule*> Scheduler::get_schedule(size_t id) {
        auto it = schedules.find(id);
        if (it != schedules.end()) {
            return &it->second;
        } else {
            return std::nullopt;
        }
    }

    void Scheduler::prepare() {
        executing = true;
        frame_index = 0;
        system_index = 0;
    }
}
