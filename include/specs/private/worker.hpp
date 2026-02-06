#pragma once

#include <specs/private/system.hpp>

#include <specs/commands.hpp>

namespace specs {
    class Worker {
    public:
        CommandQueue command_queue;

        void run() {}
    };
}
