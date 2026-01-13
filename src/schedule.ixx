module;

#include <vector>

export module specs.schedule;

namespace specs {
    struct System;

    class ScheduleTree {
    private:
        std::vector<System> systems;
        ScheduleTree* next;
    };
};