module;

#include <deque>

export module specs.commands;

import specs.world;
import specs.entity;

namespace specs {
    struct Command {};

    export class EntityCommands;

    export class Commands {
    private:
        specs::World* world;
        std::deque<Command> command_queue;
    public:
        EntityCommands spawn();
    };

    export class EntityCommands {
    private:
        EntityHandle handle;
    };
}
